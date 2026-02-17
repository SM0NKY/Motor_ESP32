#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "GPS.h"

// --- micro-ROS ---
// NOTA: Ya no incluimos headers de transporte manual.
// El componente oficial lo maneja internamente gracias al menuconfig.
#include <rmw_microros/rmw_microros.h>
#include <rmw_microros/ping.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/string.h>

// GPS UART2 pins
#define GPS_TX_PIN GPIO_NUM_17
#define GPS_RX_PIN GPIO_NUM_16
#define GPS_UART_PORT UART_NUM_2

// --- micro-ROS objects ---
static rcl_publisher_t gps_pub;
static std_msgs__msg__String msg;
static char msg_buffer[128];

static GPS* g_gps = nullptr;

// Timer callback: build + publish message
static void timer_callback(rcl_timer_t * timer, int64_t /*last_call_time*/)
{
    if (timer == NULL || g_gps == nullptr) return;

    g_gps->process_data();
    Data_GPS current = g_gps->get_Data();

    int n = 0;
    // Limpiamos buffer por seguridad
    memset(msg_buffer, 0, sizeof(msg_buffer));

    if (current.isValid) {
        n = snprintf(
            msg_buffer, sizeof(msg_buffer),
            "Lat: %.6f | Lon: %.6f | Speed: %.2f km/h | RAW_STATUS: VALID",
            current.latitude,
            current.longitude,
            current.speed_kmh
        );
    } else {
        n = snprintf(msg_buffer, sizeof(msg_buffer), "GPS INVALID (no fix yet)");
    }

    if (n < 0) return;

    // Aseguramos que el string esté terminado y asignamos tamaño
    if ((size_t)n >= sizeof(msg_buffer)) {
        n = (int)sizeof(msg_buffer) - 1;
    }
    msg_buffer[n] = '\0';

    msg.data.data = msg_buffer;
    msg.data.size = (size_t)n;
    msg.data.capacity = sizeof(msg_buffer);

    rcl_ret_t prc = rcl_publish(&gps_pub, &msg, NULL);
    (void)prc;
}

extern "C" void app_main(void)
{
    // IMPORTANTE: No usar printf() aquí porque la consola (UART0) 
    // está deshabilitada para usarla con micro-ROS.

    // Dale tiempo al sistema para estabilizarse
    vTaskDelay(pdMS_TO_TICKS(2000));

    // --- micro-ROS transport init ---
    // ELIMINADO: microros_serial_transport_init();
    // La librería inicia el UART automáticamente al llamar a rclc_support_init
    // usando la configuración que pusiste en el menuconfig.

    // Ping al agente (Bloqueante hasta que conecte)
    while (rmw_uros_ping_agent(100, 1) != RMW_RET_OK) {
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    // --- GPS init ---
    static GPS gps_UART(GPS_TX_PIN, GPS_RX_PIN, GPS_UART_PORT);
    gps_UART.init();
    g_gps = &gps_UART;

    // --- rclc init ---
    rcl_allocator_t allocator = rcl_get_default_allocator();
    rclc_support_t support;
    
    // Inicialización estándar
    rcl_ret_t rc = rclc_support_init(&support, 0, NULL, &allocator);
    if (rc != RCL_RET_OK) {
        // Si falla, bucle infinito (sin printf porque no hay consola)
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- node ---
    rcl_node_t node;
    rc = rclc_node_init_default(&node, "esp32_gps_node", "", &support);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- publisher ---
    rc = rclc_publisher_init_default(
        &gps_pub,
        &node,
        ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, String),
        "/gps/pose"
    );
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- message memory init ---
    // Inicializamos la estructura del mensaje String
    std_msgs__msg__String__init(&msg);
    // Asignamos el puntero a nuestro buffer estático
    msg.data.data = msg_buffer;
    msg.data.capacity = sizeof(msg_buffer);
    msg.data.size = 0;
    msg_buffer[0] = '\0';

    // --- timer (1 Hz) ---
    rcl_timer_t timer;
    const uint64_t period_ns = RCL_MS_TO_NS(1000);
    rc = rclc_timer_init_default(&timer, &support, period_ns, timer_callback);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- executor ---
    rclc_executor_t executor;
    rc = rclc_executor_init(&executor, &support.context, 1, &allocator);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    rc = rclc_executor_add_timer(&executor, &timer);
    if (rc != RCL_RET_OK) {
        while (1) vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // --- main loop ---
    while (1) {
        rclc_executor_spin_some(&executor, RCL_MS_TO_NS(50));
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}