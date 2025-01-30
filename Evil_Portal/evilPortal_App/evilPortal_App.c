#include <furi.h>
#include <furi_hal.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <stdlib.h>
#include <string.h>
#include <file/file.h>

#define SET_HTML_CMD "sethtml="
#define SET_AP_CMD "setap="
#define RESET_CMD "reset"
#define START_CMD "start"
#define STOP_CMD "stop"
#define ACK_CMD "ack"

static void evilPortalApp_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);
    canvas_clear(canvas);
    canvas_draw_str(canvas, 5, 10, "Evil Portal App");
    canvas_draw_str(canvas, 5, 20, "Press OK to start");
    canvas_draw_str(canvas, 5, 30, "Press UP to select HTML");
    canvas_draw_str(canvas, 5, 40, "Press DOWN to set AP");
    canvas_draw_str(canvas, 5, 50, "Press LEFT to stop server");
    canvas_draw_str(canvas, 5, 60, "Press RIGHT to reset");
}

static void evilPortalApp_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    ViewPort* view_port = ctx;

    if(input_event->type == InputTypePress) {
        if(input_event->key == InputKeyOk) {
            // Send START_CMD to ESP32S2
            furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)START_CMD, strlen(START_CMD));
            furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\n", 1);
        } else if(input_event->key == InputKeyUp) {
            // Select HTML file
            char* html_file_path = "/ext/html_files/selected.html";
            char html_content[20000];
            File* file = file_open(html_file_path, FileModeRead);
            if(file) {
                size_t read_size = file_read(file, html_content, sizeof(html_content) - 1);
                html_content[read_size] = '\0';
                file_close(file);
                // Send SET_HTML_CMD to ESP32S2
                furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)SET_HTML_CMD, strlen(SET_HTML_CMD));
                furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)html_content, strlen(html_content));
                furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\n", 1);
            }
        } else if(input_event->key == InputKeyDown) {
            // Set AP from config file
            char* ap_config_path = "/ext/ap_config.txt";
            char ap_name[30];
            File* file = file_open(ap_config_path, FileModeRead);
            if(file) {
                size_t read_size = file_read(file, ap_name, sizeof(ap_name) - 1);
                ap_name[read_size] = '\0';
                file_close(file);
                // Send SET_AP_CMD to ESP32S2
                furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)SET_AP_CMD, strlen(SET_AP_CMD));
                furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)ap_name, strlen(ap_name));
                furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\n", 1);
            }
        } else if(input_event->key == InputKeyLeft) {
            // Send STOP_CMD to ESP32S2
            furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)STOP_CMD, strlen(STOP_CMD));
            furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\n", 1);
        } else if(input_event->key == InputKeyRight) {
            // Send RESET_CMD to ESP32S2
            furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)RESET_CMD, strlen(RESET_CMD));
            furi_hal_uart_tx(FuriHalUartIdUSART1, (uint8_t*)"\n", 1);
        } else if(input_event->key == InputKeyBack) {
            // Exit the app
            view_port_stop(view_port);
        }
    }
}

int32_t evilPortalApp_main(void* p) {
    UNUSED(p);

    // Initialize UART
    furi_hal_uart_init(FuriHalUartIdUSART1, 115200);

    // Initialize GUI
    Gui* gui = furi_record_open("gui");
    ViewPort* view_port = view_port_alloc();

    view_port_draw_callback_set(view_port, evilPortalApp_draw_callback, NULL);
    view_port_input_callback_set(view_port, evilPortalApp_input_callback, view_port);

    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Main loop
    while(view_port_is_running(view_port)) {
        furi_delay_ms(100);
    }

    // Cleanup
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close("gui");

    return 0;
}
