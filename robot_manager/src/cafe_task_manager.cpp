#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "std_srvs/srv/trigger.hpp"
#include "tf2/LinearMath/Quaternion.h"

#include "robot_manager/msg/robot_status.hpp"
#include "robot_manager/srv/add_order.hpp"

using namespace std::chrono_literals;

struct NamedPose
{
  std::string name;
  double x {};
  double y {};
  double z {};
  double roll {};
  double pitch {};
  double yaw {};
};

enum class OrderStatus
{
  PREPARING,
  READY_IN_KITCHEN,
  ONBOARD,
  DELIVERED
};

enum class RobotState
{
  IDLE_AT_KITCHEN,
  GOING_TO_KITCHEN,
  WAITING_FOR_RELEASE,
  GOING_TO_TABLE,
  RETURNING_ORDERS_TO_KITCHEN,
  WAITING_BEFORE_CHARGE,
  GOING_TO_CHARGE,
  CHARGING
};

enum class NavPurpose
{
  NONE,
  TO_KITCHEN_FOR_PICKUP,
  TO_KITCHEN_RETURN_HOME,
  TO_KITCHEN_LOW_BATTERY,
  TO_TABLE_DELIVERY,
  TO_CHARGING
};

struct CafeOrder
{
  std::string order_id;
  std::string table_id;
  std::string item_name;
  OrderStatus status {OrderStatus::PREPARING};
  rclcpp::Time created_time;
  rclcpp::Time ready_time;
};

class CafeTaskManager : public rclcpp::Node
{
public:
  using NavigateToPose = nav2_msgs::action::NavigateToPose;
  using GoalHandleNav = rclcpp_action::ClientGoalHandle<NavigateToPose>;

  CafeTaskManager()
  : Node("cafe_task_manager"),
    gen_(std::random_device{}()),
    prep_dist_(5, 30),
    delivery_wait_seconds_(5)
  {
    add_order_service_ =
      this->create_service<robot_manager::srv::AddOrder>(
      "/add_order",
      std::bind(
        &CafeTaskManager::handle_add_order, this,
        std::placeholders::_1, std::placeholders::_2));

    release_service_ =
      this->create_service<std_srvs::srv::Trigger>(
      "/release_robot_from_kitchen",
      std::bind(
        &CafeTaskManager::handle_release_robot, this,
        std::placeholders::_1, std::placeholders::_2));

    status_pub_ =
      this->create_publisher<robot_manager::msg::RobotStatus>(
      "/robot_status", 10);

    nav_client_ =
      rclcpp_action::create_client<NavigateToPose>(this, "/navigate_to_pose");

    main_timer_ = this->create_wall_timer(
      500ms, std::bind(&CafeTaskManager::main_loop, this));

    initialize_locations();

    current_location_name_ = "kitchen";
    current_target_name_.clear();
    robot_state_ = RobotState::IDLE_AT_KITCHEN;
    battery_level_ = 100;
    delivered_count_ = 0;
    order_counter_ = 0;
    max_capacity_ = 3;

    nav_in_progress_ = false;
    waiting_for_release_ = false;
    low_battery_mode_ = false;
    charge_countdown_active_ = false;
    charging_active_ = false;
    charging_finish_time_ = this->now();

    RCLCPP_INFO(this->get_logger(), "[SYSTEM] Cafe Task Manager started.");
  }

private:
  rclcpp::Service<robot_manager::srv::AddOrder>::SharedPtr add_order_service_;
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr release_service_;
  rclcpp::Publisher<robot_manager::msg::RobotStatus>::SharedPtr status_pub_;
  rclcpp::TimerBase::SharedPtr main_timer_;
  rclcpp_action::Client<NavigateToPose>::SharedPtr nav_client_;

  std::map<std::string, NamedPose> locations_;

  std::vector<CafeOrder> preparing_orders_;
  std::vector<CafeOrder> ready_orders_;
  std::vector<CafeOrder> onboard_orders_;
  std::vector<CafeOrder> delivered_orders_;

  std::string current_location_name_;
  std::string current_target_name_;

  RobotState robot_state_;
  NavPurpose nav_purpose_ {NavPurpose::NONE};

  bool nav_in_progress_;
  bool waiting_for_release_;
  bool low_battery_mode_;
  bool charge_countdown_active_;
  bool charging_active_;

  int battery_level_;
  int delivered_count_;
  int order_counter_;
  int max_capacity_;

  rclcpp::Time charge_countdown_deadline_;
  rclcpp::Time charging_finish_time_;

  std::mt19937 gen_;
  std::uniform_int_distribution<int> prep_dist_;
  int delivery_wait_seconds_;

  void initialize_locations()
  {
    locations_["kitchen"] = {
      "kitchen",
      1.551, 6.37, 0.2,
      0.0, 0.023, -1.6
    };

    locations_["charging_station"] = {
      "charging_station",
      3.175, 9.33, 0.2,
      0.0, 0.023, -1.56
    };

    locations_["table_1"] = {
      "table_1",
      -1.5, -1.0, 0.2,
      0.0363, 0.0, -1.64
    };
    locations_["table_2"] = {
      "table_2",
      -1.5, -4.5, 0.2,
      0.0363, 0.0, -1.64
    };
    locations_["table_3"] = {
      "table_3",
      -1.5, -8.0, 0.2,
      0.0363, 0.0, -1.64
    };
    locations_["table_4"] = {
      "table_4",
      2.4, -1.0, 0.2,
      0.0363, 0.0, -1.64
    };
    locations_["table_5"] = {
      "table_5",
      2.4, -4.5, 0.2,
      0.0363, 0.0, -1.64
    };
    locations_["table_6"] = {
      "table_6",
      2.4, -9.0, 0.2,
      0.0363, 0.0, -1.64
    };
  }

  void main_loop()
  {
    update_preparing_orders();
    update_wait_before_charge();
    update_charging();
    publish_status();

    if (nav_in_progress_) {
      return;
    }

    if (charging_active_) {
      return;
    }

    if (charge_countdown_active_) {
      return;
    }

    if (waiting_for_release_) {
      return;
    }

    if (battery_level_ <= 20 && !low_battery_mode_) {
      start_low_battery_procedure();
      return;
    }

    if (!onboard_orders_.empty()) {
      decide_while_onboard_orders_exist();
      return;
    }

    if (!ready_orders_.empty()) {
      go_pickup_or_wait_at_kitchen();
      return;
    }

    return_home_if_needed();
  }

  void handle_add_order(
    const std::shared_ptr<robot_manager::srv::AddOrder::Request> request,
    std::shared_ptr<robot_manager::srv::AddOrder::Response> response)
  {
    if (locations_.find(request->table_id) == locations_.end()) {
      response->accepted = false;
      response->order_id = "";
      response->message = "Invalid table_id";
      RCLCPP_WARN(
        this->get_logger(),
        "[ORDER] Rejected invalid table_id: %s",
        request->table_id.c_str());
      return;
    }

    CafeOrder order;
    order.order_id = generate_order_id();
    order.table_id = request->table_id;
    order.item_name = request->item_name;
    order.status = OrderStatus::PREPARING;
    order.created_time = this->now();

    int prep_seconds = prep_dist_(gen_);
    order.ready_time = this->now() + rclcpp::Duration::from_seconds(prep_seconds);

    preparing_orders_.push_back(order);

    response->accepted = true;
    response->order_id = order.order_id;

    std::ostringstream oss;
    oss << "Order accepted. Estimated prep time: " << prep_seconds << " seconds.";
    response->message = oss.str();

    RCLCPP_INFO(
      this->get_logger(),
      "[ORDER] Added %s -> %s (%s), prep=%d sec",
      order.order_id.c_str(),
      order.table_id.c_str(),
      order.item_name.c_str(),
      prep_seconds);
  }

  void handle_release_robot(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> /*request*/,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response)
  {
    if (!waiting_for_release_) {
      response->success = false;
      response->message = "Robot is not waiting in kitchen.";
      RCLCPP_WARN(this->get_logger(), "[RELEASE] Ignored: robot is not waiting in kitchen.");
      return;
    }

    if (low_battery_mode_) {
      response->success = false;
      response->message = "Robot is in low battery procedure, release ignored.";
      RCLCPP_WARN(this->get_logger(), "[RELEASE] Ignored: robot is in low battery procedure.");
      return;
    }

    waiting_for_release_ = false;

    RCLCPP_INFO(this->get_logger(), "[RELEASE] Kitchen released robot.");

    if (current_location_name_ == "kitchen") {
      pickup_orders_at_kitchen();
    }

    if (!onboard_orders_.empty()) {
      RCLCPP_INFO(
        this->get_logger(),
        "[RELEASE] Robot leaving kitchen with %zu onboard order(s).",
        onboard_orders_.size());
      deliver_nearest_onboard_order();
      response->success = true;
      response->message = "Robot released and started delivery.";
      return;
    }

    robot_state_ = RobotState::IDLE_AT_KITCHEN;
    current_target_name_.clear();

    response->success = true;
    response->message = "Robot released, but no ready orders were available.";
    RCLCPP_INFO(this->get_logger(), "[RELEASE] No ready orders available. Robot stays idle in kitchen.");
  }

  void update_preparing_orders()
  {
    const auto now = this->now();

    std::vector<CafeOrder> still_preparing;
    still_preparing.reserve(preparing_orders_.size());

    for (auto & order : preparing_orders_) {
      if (now >= order.ready_time) {
        order.status = OrderStatus::READY_IN_KITCHEN;
        ready_orders_.push_back(order);

        RCLCPP_INFO(
          this->get_logger(),
          "[KITCHEN] Order ready: %s -> %s",
          order.order_id.c_str(),
          order.table_id.c_str());
      } else {
        still_preparing.push_back(order);
      }
    }

    preparing_orders_ = still_preparing;
  }

  void start_low_battery_procedure()
  {
    low_battery_mode_ = true;

    RCLCPP_WARN(
      this->get_logger(),
      "[BATTERY] Reached %d%%. Starting low battery procedure.",
      battery_level_);

    if (current_location_name_ != "kitchen") {
      current_target_name_ = "kitchen";
      robot_state_ = RobotState::RETURNING_ORDERS_TO_KITCHEN;
      send_navigation_goal("kitchen", NavPurpose::TO_KITCHEN_LOW_BATTERY);
      return;
    }

    handle_arrival_to_kitchen_for_low_battery();
  }

  void handle_arrival_to_kitchen_for_low_battery()
  {
    if (!onboard_orders_.empty()) {
      RCLCPP_WARN(
        this->get_logger(),
        "[BATTERY] Returning onboard orders to kitchen before charging.");
    }

    for (auto & order : onboard_orders_) {
      order.status = OrderStatus::READY_IN_KITCHEN;
      ready_orders_.push_back(order);
    }
    onboard_orders_.clear();

    charge_countdown_active_ = true;
    charge_countdown_deadline_ = this->now() + rclcpp::Duration::from_seconds(60);
    robot_state_ = RobotState::WAITING_BEFORE_CHARGE;
    current_target_name_ = "charging_station";

    RCLCPP_WARN(
      this->get_logger(),
      "[BATTERY] Waiting 60 seconds before charging to allow staff to take returned orders.");
  }

  void update_wait_before_charge()
  {
    if (!charge_countdown_active_) {
      return;
    }

    const auto now = this->now();
    if (now < charge_countdown_deadline_) {
      auto remaining = (charge_countdown_deadline_ - now).seconds();
      RCLCPP_WARN_THROTTLE(
        this->get_logger(),
        *this->get_clock(),
        5000,
        "[BATTERY] Charging in %.0f seconds",
        remaining);
      return;
    }

    charge_countdown_active_ = false;
    robot_state_ = RobotState::GOING_TO_CHARGE;
    current_target_name_ = "charging_station";

    send_navigation_goal("charging_station", NavPurpose::TO_CHARGING);
  }

  void start_charging()
  {
    charging_active_ = true;
    robot_state_ = RobotState::CHARGING;
    current_target_name_.clear();
    charging_finish_time_ = this->now() + rclcpp::Duration::from_seconds(15);

    RCLCPP_INFO(this->get_logger(), "[BATTERY] Charging started.");
  }

  void update_charging()
  {
    if (!charging_active_) {
      return;
    }

    if (this->now() < charging_finish_time_) {
      return;
    }

    battery_level_ = 100;
    charging_active_ = false;
    low_battery_mode_ = false;
    robot_state_ = RobotState::IDLE_AT_KITCHEN;

    RCLCPP_INFO(this->get_logger(), "[BATTERY] Charging completed. Battery=100%%.");

    if (current_location_name_ != "kitchen") {
      current_target_name_ = "kitchen";
      robot_state_ = RobotState::GOING_TO_KITCHEN;
      send_navigation_goal("kitchen", NavPurpose::TO_KITCHEN_RETURN_HOME);
    }
  }

  void decide_while_onboard_orders_exist()
  {
    if ((int)onboard_orders_.size() >= max_capacity_) {
      deliver_nearest_onboard_order();
      return;
    }

    if (ready_orders_.empty()) {
      deliver_nearest_onboard_order();
      return;
    }

    double delivery_distance = nearest_onboard_delivery_distance();
    double kitchen_distance = distance_between(current_location_name_, "kitchen");

    if (current_location_name_ == "kitchen") {
      waiting_for_release_ = true;
      robot_state_ = RobotState::WAITING_FOR_RELEASE;
      current_target_name_ = "kitchen";

      RCLCPP_INFO(
        this->get_logger(),
        "[KITCHEN] Robot returned to kitchen with %zu onboard order(s). Waiting for release.",
        onboard_orders_.size());
      return;
    }

    if (kitchen_distance < delivery_distance) {
      robot_state_ = RobotState::GOING_TO_KITCHEN;
      current_target_name_ = "kitchen";

      RCLCPP_INFO(this->get_logger(), "[DECISION] Kitchen is closer than next delivery. Returning to kitchen.");
      send_navigation_goal("kitchen", NavPurpose::TO_KITCHEN_FOR_PICKUP);
    } else {
      deliver_nearest_onboard_order();
    }
  }

  void go_pickup_or_wait_at_kitchen()
  {
    if (current_location_name_ == "kitchen") {
      waiting_for_release_ = true;
      robot_state_ = RobotState::WAITING_FOR_RELEASE;
      current_target_name_ = "kitchen";

      RCLCPP_INFO(
        this->get_logger(),
        "[KITCHEN] Robot already in kitchen. Waiting for release to load orders.");
      return;
    }

    robot_state_ = RobotState::GOING_TO_KITCHEN;
    current_target_name_ = "kitchen";

    RCLCPP_INFO(this->get_logger(), "[NAV] Going to kitchen for pickup.");
    send_navigation_goal("kitchen", NavPurpose::TO_KITCHEN_FOR_PICKUP);
  }

  void return_home_if_needed()
  {
    if (current_location_name_ == "kitchen") {
      robot_state_ = RobotState::IDLE_AT_KITCHEN;
      current_target_name_.clear();
      return;
    }

    robot_state_ = RobotState::GOING_TO_KITCHEN;
    current_target_name_ = "kitchen";

    RCLCPP_INFO(this->get_logger(), "[NAV] No pending work. Returning home to kitchen.");
    send_navigation_goal("kitchen", NavPurpose::TO_KITCHEN_RETURN_HOME);
  }

  void pickup_orders_at_kitchen()
  {
    if (current_location_name_ != "kitchen") {
      RCLCPP_ERROR(this->get_logger(), "[PICKUP] Called outside kitchen.");
      return;
    }

    if (ready_orders_.empty()) {
      RCLCPP_INFO(this->get_logger(), "[PICKUP] No ready orders in kitchen.");
      return;
    }

    int available_slots = max_capacity_ - static_cast<int>(onboard_orders_.size());
    if (available_slots <= 0) {
      RCLCPP_INFO(this->get_logger(), "[PICKUP] Robot already at full capacity.");
      return;
    }

    sort_ready_orders_by_distance_from_kitchen();

    RCLCPP_INFO(
      this->get_logger(),
      "[PICKUP] Start loading. Ready=%zu, onboard=%zu, free_slots=%d",
      ready_orders_.size(),
      onboard_orders_.size(),
      available_slots);

    int picked = 0;
    std::vector<CafeOrder> remaining;
    remaining.reserve(ready_orders_.size());

    for (auto & order : ready_orders_) {
      if (picked < available_slots) {
        order.status = OrderStatus::ONBOARD;
        onboard_orders_.push_back(order);
        picked++;

        RCLCPP_INFO(
          this->get_logger(),
          "[PICKUP] Loaded %s -> %s (%s)",
          order.order_id.c_str(),
          order.table_id.c_str(),
          order.item_name.c_str());
      } else {
        remaining.push_back(order);
      }
    }

    ready_orders_ = remaining;

    RCLCPP_INFO(
      this->get_logger(),
      "[PICKUP] Finished loading. Picked=%d, onboard_now=%zu, ready_remaining=%zu",
      picked,
      onboard_orders_.size(),
      ready_orders_.size());
  }

  void deliver_nearest_onboard_order()
  {
    if (onboard_orders_.empty()) {
      RCLCPP_WARN(this->get_logger(), "[DELIVERY] No onboard orders to deliver.");
      return;
    }

    int best_index = find_nearest_onboard_order_index();
    if (best_index < 0) {
      RCLCPP_ERROR(this->get_logger(), "[DELIVERY] No valid onboard order found.");
      return;
    }

    const auto & order = onboard_orders_[best_index];
    robot_state_ = RobotState::GOING_TO_TABLE;
    current_target_name_ = order.table_id;

    RCLCPP_INFO(
      this->get_logger(),
      "[DELIVERY] Heading to %s for %s (%s). Onboard=%zu",
      order.table_id.c_str(),
      order.order_id.c_str(),
      order.item_name.c_str(),
      onboard_orders_.size());

    send_navigation_goal(order.table_id, NavPurpose::TO_TABLE_DELIVERY);
  }

  void finalize_table_delivery()
  {
    int best_index = find_onboard_order_index_by_target(current_location_name_);
    if (best_index < 0) {
      RCLCPP_ERROR(
        this->get_logger(),
        "[DELIVERY] Arrived at %s but found no matching onboard order.",
        current_location_name_.c_str());
      return;
    }

    CafeOrder order = onboard_orders_[best_index];
    order.status = OrderStatus::DELIVERED;
    delivered_orders_.push_back(order);
    delivered_count_++;

    onboard_orders_.erase(onboard_orders_.begin() + best_index);

    consume_battery_after_delivery();

    RCLCPP_INFO(
      this->get_logger(),
      "[DELIVERY] Delivered %s to %s. Onboard_left=%zu, delivered_total=%d",
      order.order_id.c_str(),
      order.table_id.c_str(),
      onboard_orders_.size(),
      delivered_count_);

    current_target_name_.clear();

    RCLCPP_INFO(
      this->get_logger(),
      "[DELIVERY] Waiting %d seconds at table before next action...",
      delivery_wait_seconds_);
    rclcpp::sleep_for(std::chrono::seconds(delivery_wait_seconds_));
  }

  void send_navigation_goal(const std::string & location_name, NavPurpose purpose)
  {
    if (locations_.find(location_name) == locations_.end()) {
      RCLCPP_ERROR(this->get_logger(), "[NAV] Unknown target location: %s", location_name.c_str());
      return;
    }

    if (!nav_client_->wait_for_action_server(2s)) {
      RCLCPP_ERROR(this->get_logger(), "[NAV] NavigateToPose action server not available.");
      return;
    }

    nav_in_progress_ = true;
    nav_purpose_ = purpose;

    auto goal_msg = NavigateToPose::Goal();
    goal_msg.pose = build_pose_stamped(locations_.at(location_name));

    RCLCPP_INFO(
      this->get_logger(),
      "[NAV] Sending goal -> %s (purpose=%d)",
      location_name.c_str(),
      static_cast<int>(purpose));

    auto options = rclcpp_action::Client<NavigateToPose>::SendGoalOptions();

    options.goal_response_callback =
      [this](const GoalHandleNav::SharedPtr & goal_handle)
      {
        if (!goal_handle) {
          nav_in_progress_ = false;
          nav_purpose_ = NavPurpose::NONE;
          RCLCPP_ERROR(this->get_logger(), "[NAV] Goal rejected.");
        } else {
          RCLCPP_INFO(this->get_logger(), "[NAV] Goal accepted.");
        }
      };

    options.feedback_callback =
      [this](
      GoalHandleNav::SharedPtr,
      const std::shared_ptr<const NavigateToPose::Feedback> feedback)
      {
        if (feedback) {
          RCLCPP_INFO_THROTTLE(
            this->get_logger(),
            *this->get_clock(),
            2000,
            "[NAV] Distance remaining: %.2f",
            feedback->distance_remaining);
        }
      };

    options.result_callback =
      [this, location_name](const GoalHandleNav::WrappedResult & result)
      {
        nav_in_progress_ = false;

        if (result.code == rclcpp_action::ResultCode::SUCCEEDED) {
          current_location_name_ = location_name;
          RCLCPP_INFO(
            this->get_logger(),
            "[NAV] Succeeded. Arrived at %s",
            current_location_name_.c_str());
          handle_navigation_success();
        } else {
          RCLCPP_ERROR(
            this->get_logger(),
            "[NAV] Failed to %s",
            location_name.c_str());
          handle_navigation_failure();
        }

        nav_purpose_ = NavPurpose::NONE;
      };

    nav_client_->async_send_goal(goal_msg, options);
  }

  geometry_msgs::msg::PoseStamped build_pose_stamped(const NamedPose & pose)
  {
    geometry_msgs::msg::PoseStamped msg;
    msg.header.frame_id = "map";
    msg.header.stamp = this->now();

    msg.pose.position.x = pose.x;
    msg.pose.position.y = pose.y;
    msg.pose.position.z = pose.z;

    tf2::Quaternion q;
    q.setRPY(pose.roll, pose.pitch, pose.yaw);
    q.normalize();

    msg.pose.orientation.x = q.x();
    msg.pose.orientation.y = q.y();
    msg.pose.orientation.z = q.z();
    msg.pose.orientation.w = q.w();

    return msg;
  }

  void handle_navigation_success()
  {
    switch (nav_purpose_) {
      case NavPurpose::TO_KITCHEN_FOR_PICKUP:
        waiting_for_release_ = true;
        robot_state_ = RobotState::WAITING_FOR_RELEASE;
        current_target_name_ = "kitchen";
        RCLCPP_INFO(this->get_logger(), "[KITCHEN] Arrived at kitchen. Waiting for release.");
        break;

      case NavPurpose::TO_KITCHEN_RETURN_HOME:
        robot_state_ = RobotState::IDLE_AT_KITCHEN;
        current_target_name_.clear();
        RCLCPP_INFO(this->get_logger(), "[KITCHEN] Returned home to kitchen.");
        break;

      case NavPurpose::TO_KITCHEN_LOW_BATTERY:
        handle_arrival_to_kitchen_for_low_battery();
        break;

      case NavPurpose::TO_TABLE_DELIVERY:
        finalize_table_delivery();
        current_target_name_.clear();
        break;

      case NavPurpose::TO_CHARGING:
        start_charging();
        break;

      case NavPurpose::NONE:
      default:
        break;
    }
  }

  void handle_navigation_failure()
  {
    current_target_name_.clear();

    if (low_battery_mode_) {
      robot_state_ = RobotState::WAITING_BEFORE_CHARGE;
      return;
    }

    if (current_location_name_ == "kitchen") {
      robot_state_ = RobotState::IDLE_AT_KITCHEN;
    } else {
      robot_state_ = RobotState::GOING_TO_KITCHEN;
    }
  }

  std::string generate_order_id()
  {
    ++order_counter_;
    std::ostringstream oss;
    oss << "ORD_" << order_counter_;
    return oss.str();
  }

  double distance_between(const std::string & a, const std::string & b) const
  {
    const auto it_a = locations_.find(a);
    const auto it_b = locations_.find(b);

    if (it_a == locations_.end() || it_b == locations_.end()) {
      return std::numeric_limits<double>::max();
    }

    const double dx = it_a->second.x - it_b->second.x;
    const double dy = it_a->second.y - it_b->second.y;
    return std::sqrt(dx * dx + dy * dy);
  }

  double nearest_onboard_delivery_distance() const
  {
    if (onboard_orders_.empty()) {
      return std::numeric_limits<double>::max();
    }

    double best = std::numeric_limits<double>::max();

    for (const auto & order : onboard_orders_) {
      double d = distance_between(current_location_name_, order.table_id);
      if (d < best) {
        best = d;
      }
    }

    return best;
  }

  int find_nearest_onboard_order_index() const
  {
    if (onboard_orders_.empty()) {
      return -1;
    }

    int best_index = -1;
    double best_distance = std::numeric_limits<double>::max();

    for (size_t i = 0; i < onboard_orders_.size(); ++i) {
      double d = distance_between(current_location_name_, onboard_orders_[i].table_id);
      if (d < best_distance) {
        best_distance = d;
        best_index = static_cast<int>(i);
      }
    }

    return best_index;
  }

  int find_onboard_order_index_by_target(const std::string & target_table) const
  {
    for (size_t i = 0; i < onboard_orders_.size(); ++i) {
      if (onboard_orders_[i].table_id == target_table) {
        return static_cast<int>(i);
      }
    }
    return -1;
  }

  void sort_ready_orders_by_distance_from_kitchen()
  {
    std::sort(
      ready_orders_.begin(),
      ready_orders_.end(),
      [this](const CafeOrder & a, const CafeOrder & b)
      {
        return distance_between("kitchen", a.table_id) <
               distance_between("kitchen", b.table_id);
      });
  }

  void consume_battery_after_delivery()
  {
    battery_level_ -= 10;
    if (battery_level_ < 0) {
      battery_level_ = 0;
    }

    RCLCPP_INFO(
      this->get_logger(),
      "[BATTERY] After delivery: %d%%",
      battery_level_);
  }

  std::string robot_state_to_string() const
  {
    switch (robot_state_) {
      case RobotState::IDLE_AT_KITCHEN:
        return "IDLE_AT_KITCHEN";
      case RobotState::GOING_TO_KITCHEN:
        return "GOING_TO_KITCHEN";
      case RobotState::WAITING_FOR_RELEASE:
        return "WAITING_FOR_RELEASE";
      case RobotState::GOING_TO_TABLE:
        return "GOING_TO_TABLE";
      case RobotState::RETURNING_ORDERS_TO_KITCHEN:
        return "RETURNING_ORDERS_TO_KITCHEN";
      case RobotState::WAITING_BEFORE_CHARGE:
        return "WAITING_BEFORE_CHARGE";
      case RobotState::GOING_TO_CHARGE:
        return "GOING_TO_CHARGE";
      case RobotState::CHARGING:
        return "CHARGING";
      default:
        return "UNKNOWN";
    }
  }

  std::vector<std::string> extract_order_ids(const std::vector<CafeOrder> & orders) const
  {
    std::vector<std::string> ids;
    ids.reserve(orders.size());

    for (const auto & order : orders) {
      ids.push_back(order.order_id);
    }

    return ids;
  }

  std::string waiting_reason() const
  {
    if (waiting_for_release_) {
      return "Waiting for kitchen release signal";
    }

    if (charge_countdown_active_) {
      return "Waiting before charge after returning onboard orders";
    }

    if (charging_active_) {
      return "Charging in progress";
    }

    return "";
  }

  void publish_status()
  {
    robot_manager::msg::RobotStatus msg;
    msg.robot_state = robot_state_to_string();
    msg.current_location_name = current_location_name_;
    msg.current_target_name = current_target_name_;
    msg.waiting_reason = waiting_reason();

    msg.battery_level = battery_level_;
    msg.preparing_count = static_cast<int>(preparing_orders_.size());
    msg.ready_count = static_cast<int>(ready_orders_.size());
    msg.onboard_count = static_cast<int>(onboard_orders_.size());
    msg.delivered_count = delivered_count_;

    msg.waiting_for_release = waiting_for_release_;
    msg.low_battery_mode = low_battery_mode_;

    msg.preparing_order_ids = extract_order_ids(preparing_orders_);
    msg.ready_order_ids = extract_order_ids(ready_orders_);
    msg.onboard_order_ids = extract_order_ids(onboard_orders_);
    msg.delivered_order_ids = extract_order_ids(delivered_orders_);

    status_pub_->publish(msg);
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<CafeTaskManager>());
  rclcpp::shutdown();
  return 0;
}