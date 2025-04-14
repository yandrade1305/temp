#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "inc/ssd1306.h"

// Definições de hardware
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;
#define TEMP_SENSOR_ADC_INPUT 4  // Canal ADC para sensor interno

// Inicialização do ADC para sensor de temperatura
void adc_init_temp_sensor() {
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(TEMP_SENSOR_ADC_INPUT);
}

// Leitura e conversão da temperatura
float read_internal_temperature() {
    // Lê valor bruto do ADC (12 bits, 0-4095)
    const uint16_t adc_value = adc_read();
    
    // Fórmula de conversão baseada na documentação do RP2040
    // Tensão de referência ADC: 3.3V, resolução 12 bits
    const float voltage = adc_value * 3.3f / (1 << 12);
    
    // Conversão para temperatura (fórmula do datasheet)
    // Temperatura em °C = 27 - (Vmedido - 0.706)/0.001721
    return 27.0f - (voltage - 0.706f) / 0.001721f;
}

int main() {
    stdio_init_all();

    // Inicialização do ADC para sensor de temperatura
    adc_init_temp_sensor();

    // Configuração do I2C para o display OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display OLED
    ssd1306_init();
    struct render_area frame_area = {
        start_column: 0,
        end_column: ssd1306_width - 1,
        start_page: 0,
        end_page: ssd1306_n_pages - 1
    };
    calculate_render_area_buffer_length(&frame_area);
    
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);
    render_on_display(ssd, &frame_area);

    // Variáveis para temperatura
    float temperature;
    char temp_str[16];
    char title_str[] = "Temp. Interna RP2040";

    while (true) {
        // Limpa o buffer do display
        memset(ssd, 0, ssd1306_buffer_length);

        // Lê a temperatura
        temperature = read_internal_temperature();

        // Converte temperatura para string com 1 casa decimal
        snprintf(temp_str, sizeof(temp_str), "%.1f C", temperature);

        // Desenha no display
        ssd1306_draw_string(ssd, 10, 10, title_str);
        ssd1306_draw_string(ssd, 40, 30, temp_str);

        // Atualiza o display
        render_on_display(ssd, &frame_area);

        // Pequena pausa entre leituras (1 segundo)
        sleep_ms(1000);
    }

    return 0;
}