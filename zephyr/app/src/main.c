#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/promiscuous.h>
#include <zephyr/net/ieee802154.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#if defined(CONFIG_SOC_SERIES_ESP32) || defined(CONFIG_ARCH_ESP32) || defined(CONFIG_WIFI_ESP32)
#include <esp_wifi.h>
#define HAS_CUSTOM_ESP_PROMISC 1
#endif

struct net_pkt *pkt;
struct net_if *iface;
struct net_if *iface_802154;

int enable_promisc(struct net_if *iface) {
    if (!iface) {
        LOG_ERR("Interface pointer is null");
        return -EINVAL;
    }
#if defined(HAS_CUSTOM_ESP_PROMISC)
    LOG_INF("Enabling ESP-IDF promiscuous mode...");
    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_ALL
    };
    esp_wifi_set_promiscuous_filter(&filter);

    esp_err_t ret = esp_wifi_set_promiscuous(true);
    if (ret != ESP_OK) {
        LOG_ERR("Failed to enable ESP-IDF promiscuous mode: %d", ret);
        return -EIO;
    }
#else
    int ret = net_promisc_mode_on(iface);
    if (ret < 0 && ret != -EALREADY) {
        LOG_ERR("Failed to enable promsc mode (err: %d)", ret);
        return ret;
    }
#endif
    LOG_INF("Promisc mode enabled on interface");
    return ret; 
}

int main(void) {
    iface = net_if_get_default();
    iface_802154 = net_if_get_ieee802154();

    int count=0;
    
    while (count < 10) {
        k_msleep(350);
        LOG_INF("FR0G 05 v0.2alpha Initializing! Booting on %s", CONFIG_BOARD);
        count++;
    }
    LOG_INF("Starting fr0g 05 wifi sniffing module...");
    if (!iface) {
        LOG_ERR("No default network device found");
    } else {
        LOG_INF("Default network device found");
    }
    if (!iface_802154) {
        LOG_ERR("No 802154 network device found");
    } else {
        LOG_INF("802.15.4 network device found");
    }
    if (!iface && !iface_802154) {
        LOG_ERR("No network devices found");
        return 1;
    }

    LOG_DBG("net_if: %p", (void *)iface);
    LOG_DBG("net_if_802154: %p", (void *)iface);
    LOG_INF("Enabling promisc mode on network interface...");
    enable_promisc(iface);
    LOG_INF("Enabling promisc mode on network interface...");
    enable_promisc(iface);
    return 0;
}
