import time
import threading
from pathlib import Path
import webbrowser

import cv2
from cv_bridge import CvBridge
from flask import Flask, Response, jsonify, render_template, request
import rclpy
from rclpy.node import Node
from ament_index_python.packages import get_package_share_directory

from std_srvs.srv import Trigger
from sensor_msgs.msg import Image
from robot_manager.srv import AddOrder
from robot_manager.msg import RobotStatus


class RobotWebBridge(Node):
    def __init__(self):
        super().__init__("robot_web_bridge")

        self.latest_status = {
            "robot_state": "UNKNOWN",
            "current_location_name": "",
            "current_target_name": "",
            "waiting_reason": "",
            "battery_level": 0,
            "preparing_count": 0,
            "ready_count": 0,
            "onboard_count": 0,
            "delivered_count": 0,
            "waiting_for_release": False,
            "low_battery_mode": False,
            "preparing_order_ids": [],
            "ready_order_ids": [],
            "onboard_order_ids": [],
            "delivered_order_ids": [],
        }

        self.bridge = CvBridge()
        self.latest_camera_jpeg = None

        self.status_sub = self.create_subscription(
            RobotStatus, "/robot_status", self.status_callback, 10
        )

        self.camera_sub = self.create_subscription(
            Image, "/camera", self.camera_callback, 10
        )

        self.add_order_client = self.create_client(AddOrder, "/add_order")
        self.release_client = self.create_client(Trigger, "/release_robot_from_kitchen")

        self.get_logger().info("Robot Web Bridge started.")

    def status_callback(self, msg: RobotStatus):
        self.latest_status = {
            "robot_state": msg.robot_state,
            "current_location_name": msg.current_location_name,
            "current_target_name": msg.current_target_name,
            "waiting_reason": msg.waiting_reason,
            "battery_level": msg.battery_level,
            "preparing_count": msg.preparing_count,
            "ready_count": msg.ready_count,
            "onboard_count": msg.onboard_count,
            "delivered_count": msg.delivered_count,
            "waiting_for_release": msg.waiting_for_release,
            "low_battery_mode": msg.low_battery_mode,
            "preparing_order_ids": list(msg.preparing_order_ids),
            "ready_order_ids": list(msg.ready_order_ids),
            "onboard_order_ids": list(msg.onboard_order_ids),
            "delivered_order_ids": list(msg.delivered_order_ids),
        }

    def camera_callback(self, msg: Image):
        try:
            frame = self.bridge.imgmsg_to_cv2(msg, desired_encoding="bgr8")
            success, buffer = cv2.imencode(
                ".jpg", frame, [int(cv2.IMWRITE_JPEG_QUALITY), 60]
            )
            if success:
                self.latest_camera_jpeg = buffer.tobytes()
        except Exception as e:
            self.get_logger().warn(f"Camera conversion failed: {e}")

    def call_add_order(self, table_id: str, item_name: str):
        if not self.add_order_client.wait_for_service(timeout_sec=3.0):
            return {"success": False, "message": "Add order service not available."}

        req = AddOrder.Request()
        req.table_id = table_id
        req.item_name = item_name

        future = self.add_order_client.call_async(req)
        result = self.wait_for_future(future)

        if result is None:
            return {"success": False, "message": "Timeout."}

        return {
            "success": bool(result.accepted),
            "message": result.message,
            "order_id": result.order_id,
        }

    def call_release_robot(self):
        if not self.release_client.wait_for_service(timeout_sec=3.0):
            return {"success": False, "message": "Release service not available."}

        future = self.release_client.call_async(Trigger.Request())
        result = self.wait_for_future(future)

        if result is None:
            return {"success": False, "message": "Timeout."}

        return {"success": bool(result.success), "message": result.message}

    def wait_for_future(self, future, timeout_sec=3.0):
        start = time.time()
        while not future.done():
            if time.time() - start > timeout_sec:
                return None
            time.sleep(0.05)
        return future.result()


def create_app(bridge_node: RobotWebBridge):
    share_dir = Path(get_package_share_directory("robot_manager_interface"))
    template_dir = share_dir / "templates"
    static_dir = share_dir / "static"

    app = Flask(
        __name__,
        template_folder=str(template_dir),
        static_folder=str(static_dir),
    )

    @app.route("/")
    def index():
        return render_template("index.html", status_data=bridge_node.latest_status)

    @app.route("/status")
    def status():
        return jsonify(bridge_node.latest_status)

    @app.route("/add_order", methods=["POST"])
    def add_order():
        data = request.get_json(force=True)
        return jsonify(
            bridge_node.call_add_order(data.get("table_id"), data.get("item_name"))
        )

    @app.route("/release", methods=["POST"])
    def release():
        return jsonify(bridge_node.call_release_robot())

    @app.route("/camera_feed")
    def camera_feed():
        if bridge_node.latest_camera_jpeg is None:
            return ("", 204)

        return Response(
            bridge_node.latest_camera_jpeg,
            mimetype="image/jpeg",
        )

    return app


def ros_spin(node):
    rclpy.spin(node)


def open_browser():
    webbrowser.open("http://localhost:5000")


def main(args=None):
    rclpy.init(args=args)

    bridge_node = RobotWebBridge()

    ros_thread = threading.Thread(target=ros_spin, args=(bridge_node,), daemon=True)
    ros_thread.start()

    app = create_app(bridge_node)

    threading.Timer(2.0, open_browser).start()

    print("\n" + "=" * 50)
    print("🚀 Cafe Robot Web Interface Running:")
    print("👉 http://localhost:5000")
    print("=" * 50 + "\n")

    app.run(host="0.0.0.0", port=5000, debug=False)

    bridge_node.destroy_node()
    rclpy.shutdown()


if __name__ == "__main__":
    main()
