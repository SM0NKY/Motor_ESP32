// micro_ros_serial_transport.cpp
#include "micro_ros_serial_transport.hpp"

#include <cstdint>

#include "driver/uart.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <rmw_microros/rmw_microros.h>

// UART0 para micro-ROS (USB-UART típico)
static constexpr uart_port_t UROS_UART = UART_NUM_0;
static constexpr int UROS_BAUDRATE = 115200;
static constexpr int UROS_BUF_SIZE = 2048;

static bool esp32_serial_open(struct uxrCustomTransport * transport)
{
  (void)transport;

  // Config completa (evita warnings y es más estable entre versiones IDF)
  uart_config_t cfg = {};
  cfg.baud_rate = UROS_BAUDRATE;
  cfg.data_bits = UART_DATA_8_BITS;
  cfg.parity    = UART_PARITY_DISABLE;
  cfg.stop_bits = UART_STOP_BITS_1;
  cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
  cfg.source_clk = UART_SCLK_DEFAULT;
  cfg.rx_flow_ctrl_thresh = 0;

  // Ahora que la consola está en UART1, UART0 debe estar libre:
  // Forzamos estado limpio para evitar "driver ya instalado" con config vieja.
  (void)uart_driver_delete(UROS_UART);

  if (uart_param_config(UROS_UART, &cfg) != ESP_OK) {
    return false;
  }

  // UART0 usa pines por defecto (TX=GPIO1, RX=GPIO3).
  // No tocamos pines para no pelear con el USB-UART bridge.
  // Si quieres forzar explícito, puedes usar uart_set_pin con GPIO1/3.
  // (void)uart_set_pin(UROS_UART, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
  //                    UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

  // Instalar driver con RX y TX buffer (TX buffer ayuda a estabilidad)
  esp_err_t install_rc = uart_driver_install(
    UROS_UART,
    UROS_BUF_SIZE,   // rx buffer
    UROS_BUF_SIZE,   // tx buffer
    0,
    nullptr,
    0
  );

  if (install_rc != ESP_OK) {
    return false;
  }

  uart_flush_input(UROS_UART);
  vTaskDelay(pdMS_TO_TICKS(50));
  return true;
}

static bool esp32_serial_close(struct uxrCustomTransport * transport)
{
  (void)transport;

  uart_flush(UROS_UART);

  // Puedes dejar el driver instalado.
  // Si quisieras liberarlo:
  // (void)uart_driver_delete(UROS_UART);

  return true;
}

static size_t esp32_serial_write(
  struct uxrCustomTransport* transport,
  const uint8_t* buf,
  size_t len,
  uint8_t* err)
{
  (void)transport;

  int written = uart_write_bytes(
    UROS_UART,
    reinterpret_cast<const char*>(buf),
    static_cast<int>(len)
  );

  if (written < 0) {
    if (err) *err = 1;
    return 0;
  }

  if (err) *err = 0;
  return static_cast<size_t>(written);
}

static size_t esp32_serial_read(
  struct uxrCustomTransport* transport,
  uint8_t* buf,
  size_t len,
  int timeout,
  uint8_t* err)
{
  (void)transport;

  int rd = uart_read_bytes(
    UROS_UART,
    buf,
    static_cast<uint32_t>(len),
    pdMS_TO_TICKS(timeout)
  );

  if (rd < 0) {
    if (err) *err = 1;
    return 0;
  }

  if (err) *err = 0;
  return static_cast<size_t>(rd);
}

extern "C" void microros_serial_transport_init(void)
{
  rmw_uros_set_custom_transport(
    true,
    nullptr,
    esp32_serial_open,
    esp32_serial_close,
    esp32_serial_write,
    esp32_serial_read
  );
}
