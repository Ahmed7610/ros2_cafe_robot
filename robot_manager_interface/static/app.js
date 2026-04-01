function updateMissionText(status) {
  const mission = document.getElementById("robot-mission");

  if (status.low_battery_mode) {
    mission.textContent = "Robot is handling low battery procedure.";
    return;
  }

  if (status.waiting_for_release) {
    mission.textContent = "Robot is waiting in kitchen for release.";
    return;
  }

  if (status.current_target_name) {
    mission.textContent = `Robot is heading to ${status.current_target_name.replace("_", " ")}.`;
    return;
  }

  if (status.current_location_name) {
    mission.textContent = `Robot is currently at ${status.current_location_name.replace("_", " ")}.`;
    return;
  }

  mission.textContent = "Robot is idle in kitchen.";
}

function updateBattery(status) {
  const battery = status.battery_level || 0;
  const batteryText = document.getElementById("battery-text");
  const batteryFill = document.getElementById("battery-fill");
  const batteryHealth = document.getElementById("battery-health-text");

  batteryText.textContent = `${battery}%`;
  batteryFill.style.width = `${battery}%`;

  if (battery <= 20) {
    batteryFill.style.background = "linear-gradient(90deg, #ef4444 0%, #dc2626 100%)";
    batteryHealth.textContent = "Low battery mode may be active";
  } else if (battery <= 50) {
    batteryFill.style.background = "linear-gradient(90deg, #f59e0b 0%, #d97706 100%)";
    batteryHealth.textContent = "Battery at medium level";
  } else {
    batteryFill.style.background = "linear-gradient(90deg, #22c55e 0%, #16a34a 100%)";
    batteryHealth.textContent = "Battery level is healthy";
  }
}

function renderOrderList(containerId, items) {
  const container = document.getElementById(containerId);
  container.innerHTML = "";

  if (!items || items.length === 0) {
    container.innerHTML = '<div class="empty-state">No orders</div>';
    return;
  }

  items.forEach(item => {
    const chip = document.createElement("div");
    chip.className = "order-chip";
    chip.textContent = item;
    container.appendChild(chip);
  });
}

function updateStatusUI(status) {
  document.getElementById("robot-state").textContent = status.robot_state || "UNKNOWN";
  document.getElementById("current-location").textContent = status.current_location_name || "--";
  document.getElementById("current-target").textContent = status.current_target_name || "--";
  document.getElementById("waiting-reason").textContent = status.waiting_reason || "--";
  document.getElementById("waiting-for-release").textContent = status.waiting_for_release ? "Yes" : "No";
  document.getElementById("low-battery-mode").textContent = status.low_battery_mode ? "Yes" : "No";

  document.getElementById("state-chip").textContent = status.robot_state || "UNKNOWN";
  document.getElementById("location-chip").textContent =
    `Location: ${status.current_location_name || "--"}`;

  document.getElementById("preparing-count").textContent = status.preparing_count ?? 0;
  document.getElementById("ready-count").textContent = status.ready_count ?? 0;
  document.getElementById("onboard-count").textContent = status.onboard_count ?? 0;
  document.getElementById("delivered-count").textContent = status.delivered_count ?? 0;

  document.getElementById("preparing-count-pill").textContent = status.preparing_count ?? 0;
  document.getElementById("ready-count-pill").textContent = status.ready_count ?? 0;
  document.getElementById("onboard-count-pill").textContent = status.onboard_count ?? 0;
  document.getElementById("delivered-count-pill").textContent = status.delivered_count ?? 0;

  document.getElementById("robot-load").textContent = `${status.onboard_count ?? 0} / 3`;
  document.getElementById("kitchen-load").textContent = `${status.ready_count ?? 0} orders`;

  const totalOrders =
    (status.preparing_count ?? 0) +
    (status.ready_count ?? 0) +
    (status.onboard_count ?? 0) +
    (status.delivered_count ?? 0);

  document.getElementById("total-orders").textContent = totalOrders;

  const kitchenMessage = document.getElementById("kitchen-status-message");
  const releaseBtn = document.getElementById("release-btn");

  if (status.waiting_for_release) {
    kitchenMessage.textContent = "Robot is waiting in kitchen for release.";
    releaseBtn.disabled = false;
  } else {
    kitchenMessage.textContent = "Robot is not waiting in kitchen.";
    releaseBtn.disabled = true;
  }

  updateMissionText(status);
  updateBattery(status);

  renderOrderList("preparing-orders", status.preparing_order_ids);
  renderOrderList("ready-orders", status.ready_order_ids);
  renderOrderList("onboard-orders", status.onboard_order_ids);
  renderOrderList("delivered-orders", status.delivered_order_ids);
}

async function fetchStatus() {
  try {
    const response = await fetch("/status");
    if (!response.ok) return;
    const status = await response.json();
    updateStatusUI(status);
  } catch (error) {
    console.error("Failed to fetch status:", error);
  }
}

async function submitOrder(event) {
  event.preventDefault();

  const tableId = document.getElementById("table-select").value;
  const itemName = document.getElementById("item-input").value.trim();
  const feedback = document.getElementById("order-feedback");

  if (!itemName) {
    feedback.textContent = "Please enter an item name.";
    return;
  }

  try {
    const response = await fetch("/add_order", {
      method: "POST",
      headers: {
        "Content-Type": "application/json"
      },
      body: JSON.stringify({
        table_id: tableId,
        item_name: itemName
      })
    });

    const result = await response.json();
    feedback.textContent = result.message || "Order request processed.";

    if (result.success) {
      document.getElementById("item-input").value = "";
      fetchStatus();
    }
  } catch (error) {
    feedback.textContent = "Failed to add order.";
    console.error(error);
  }
}

async function releaseRobot() {
  const feedback = document.getElementById("release-feedback");

  try {
    const response = await fetch("/release", {
      method: "POST"
    });

    const result = await response.json();
    feedback.textContent = result.message || "Release request processed.";
    fetchStatus();
  } catch (error) {
    feedback.textContent = "Failed to release robot.";
    console.error(error);
  }
}

function refreshCamera() {
  const img = document.getElementById("camera-feed");
  const placeholder = document.getElementById("camera-placeholder");

  const timestamp = new Date().getTime();
  img.src = `/camera_feed?t=${timestamp}`;

  img.onload = () => {
    placeholder.style.display = "none";
  };

  img.onerror = () => {
    placeholder.style.display = "grid";
  };
}

document.addEventListener("DOMContentLoaded", () => {
  if (window.initialStatus) {
    updateStatusUI(window.initialStatus);
  }

  document.getElementById("add-order-form").addEventListener("submit", submitOrder);
  document.getElementById("release-btn").addEventListener("click", releaseRobot);

  fetchStatus();
  refreshCamera();

  setInterval(fetchStatus, 1000);
  setInterval(refreshCamera, 200);
});