// Copyright 2015-2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_eth.h"

#include "myPhy.h"


/* Value of MII_PHY_IDENTIFIER_REGs for Microchip KSZ8081
 * (Except for bottom 4 bits of ID2, used for model revision)           // NOTE: all these registers are for the lan8720... they need to be changed in order to work for ksz8081
 */
#define KSZ8081_PHY_ID1 0x0022
#define KSZ8081_PHY_ID2 0x1560
#define KSZ8081_PHY_ID2_MASK 0xFFF0




static const char *TAG = "KSZ8081";

void phy_KSZ8081_check_phy_init(void)
{
    phy_KSZ8081_dump_registers();
    ESP_LOGI(TAG, "start py init check");

   // esp_eth_smi_wait_set(MII_BASIC_MODE_STATUS_REG, MII_AUTO_NEGOTIATION_COMPLETE, 1000);
  //  esp_eth_smi_wait_set(PHY_SPECIAL_CONTROL_STATUS_REG, AUTO_NEGOTIATION_DONE, 1000);
    ESP_LOGI(TAG, "py init check done");
}

eth_speed_mode_t phy_KSZ8081_get_speed_mode(void)
{
    return ETH_SPEED_MODE_100M;
    // if (esp_eth_smi_read(PHY_SPECIAL_CONTROL_STATUS_REG) & SPEED_INDICATION_100T) {
    //     ESP_LOGD(TAG, "phy_KSZ8081_get_speed_mode(100)");
    //     return ETH_SPEED_MODE_100M;
    // } else {
    //     ESP_LOGD(TAG, "phy_KSZ8081_get_speed_mode(10)");
    //     return ETH_SPEED_MODE_10M;
    // }
}

eth_duplex_mode_t phy_KSZ8081_get_duplex_mode(void)
{
    return ETH_MODE_FULLDUPLEX;
    // if (esp_eth_smi_read(PHY_SPECIAL_CONTROL_STATUS_REG) & DUPLEX_INDICATION_FULL) {
    //     ESP_LOGD(TAG, "phy_KSZ8081_get_duplex_mode(FULL)");
    //     return ETH_MODE_FULLDUPLEX;
    // } else {
    //     ESP_LOGD(TAG, "phy_KSZ8081_get_duplex_mode(HALF)");
    //     return ETH_MODE_HALFDUPLEX;
    // }
}

void phy_KSZ8081_power_enable(bool enable)
{
    // if (enable) {
    //     esp_eth_smi_write(SW_STRAP_CONTROL_REG, DEFAULT_STRAP_CONFIG | SW_STRAP_CONFIG_DONE);
    //     // TODO: only enable if config.flow_ctrl_enable == true
    //     phy_mii_enable_flow_ctrl();
    // }
}

esp_err_t phy_KSZ8081_init(void)
{
    ESP_LOGD(TAG, "phy_KSZ8081_init()");
    phy_KSZ8081_dump_registers();

    //esp_eth_smi_write(MII_BASIC_MODE_CONTROL_REG, MII_SOFTWARE_RESET);

    esp_err_t res1, res2;
    // Call esp_eth_smi_wait_value() with a timeout so it prints an error periodically
    res1 = esp_eth_smi_wait_value(0x02, KSZ8081_PHY_ID1, UINT16_MAX, 1000);
    res2 = esp_eth_smi_wait_value(0x03, KSZ8081_PHY_ID2, KSZ8081_PHY_ID2_MASK, 1000);

    // esp_eth_smi_write(SW_STRAP_CONTROL_REG,
    //                   DEFAULT_STRAP_CONFIG | SW_STRAP_CONFIG_DONE);

    ets_delay_us(300);

    // TODO: only enable if config.flow_ctrl_enable == true
    phy_mii_enable_flow_ctrl();

    if (res1 == ESP_OK && res2 == ESP_OK) {
        return ESP_OK;
    } else {
        return ESP_ERR_TIMEOUT;
    }
}

const eth_config_t phy_KSZ8081_default_ethernet_config = {
    // By default, the PHY address is 0 or 1 based on PHYAD0
    // pin. Can also be overriden in software. See datasheet
    // for defaults.
    .phy_addr = 0,
    .mac_mode = ETH_MODE_RMII,
    .clock_mode = ETH_CLOCK_GPIO0_IN,
    //Only FULLDUPLEX mode support flow ctrl now!
    .flow_ctrl_enable = true,
    .phy_init = phy_KSZ8081_init,
    .phy_check_init = phy_KSZ8081_check_phy_init,
    .phy_power_enable = phy_KSZ8081_power_enable,
    .phy_check_link = phy_mii_check_link_status,
    .phy_get_speed_mode = phy_KSZ8081_get_speed_mode,
    .phy_get_duplex_mode = phy_KSZ8081_get_duplex_mode,
    .phy_get_partner_pause_enable = phy_mii_get_partner_pause_enable,
    .reset_timeout_ms = 1000
};

void phy_KSZ8081_dump_registers()
{
    ESP_LOGI(TAG, "KSZ8081 Registers: (uncomment in code to actually read them)(myPhy.c)");
    // ESP_LOGI(TAG, "0     0x%04x", esp_eth_smi_read(0x0));
    // ESP_LOGI(TAG, "1     0x%04x", esp_eth_smi_read(0x1));
    // ESP_LOGI(TAG, "2     0x%04x", esp_eth_smi_read(0x2));
    // ESP_LOGI(TAG, "3     0x%04x", esp_eth_smi_read(0x3));
    // ESP_LOGI(TAG, "4     0x%04x", esp_eth_smi_read(0x4));
    // ESP_LOGI(TAG, "5     0x%04x", esp_eth_smi_read(0x5));
    // ESP_LOGI(TAG, "6     0x%04x", esp_eth_smi_read(0x6));
    // ESP_LOGI(TAG, "17    0x%04x", esp_eth_smi_read(0x17));
    // ESP_LOGI(TAG, "18    0x%04x", esp_eth_smi_read(0x18));
    // ESP_LOGI(TAG, "26    0x%04x", esp_eth_smi_read(0x26));
    // ESP_LOGI(TAG, "27    0x%04x", esp_eth_smi_read(0x27));
    // ESP_LOGI(TAG, "29    0x%04x", esp_eth_smi_read(0x29));
    // ESP_LOGI(TAG, "30    0x%04x", esp_eth_smi_read(0x30));
    // ESP_LOGI(TAG, "31    0x%04x", esp_eth_smi_read(0x31));
    
}