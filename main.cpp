/*
 * Copyright (c) 2006-2020 Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 ***********************************
 *UDP send receive example
 * A microcontroller board and MOD WIFI ESP8266
 * https://os.mbed.com/docs/mbed-os/v6.3/apis/connectivity.html
 * https://os.mbed.com/docs/mbed-os/v6.3/apis/ethernet.html
 * https://os.mbed.com/docs/mbed-os/v6.2/apis/wi-fi.html
 * https://os.mbed.com/teams/ESP8266/code/esp8266-driver/
 * https://www.olimex.com/Products/IoT/ESP8266/MOD-WIFI-ESP8266/open-source-hardware
 * https://os.mbed.com/docs/mbed-os/v6.3/apis/udpsocket.html
 *
 * L432KC --- MOD WIFI ESP8266 from OLIMEX
 * L432KC D5=PB6=UART1TX --- 3 RXD
 * L432KC D4=PB7=UART1RX --- 4 TXD
 * L432KC 3V3 --- 1 3.3V
 * L432KC GND --- 2 GND
 *
 * UDP Send Receive App on smart phone needed for testing
 *  or Windows computer with an UDP client app
 * Timo Karppinen 24.9.2020  Apache-2.0
 ***********************************/

#include "mbed.h"

// MOD WIFI ESP8266
#include "ESP8266Interface.h"   // included in the OS6


#define REMOTE_PORT 5000
#define LOCAL_PORT 5001
#define BUFF_SIZE 128


// Network interface 
ESP8266Interface esp(MBED_CONF_APP_ESP_TX_PIN, MBED_CONF_APP_ESP_RX_PIN);

SocketAddress clientUDP;  // Client on remote device
UDPSocket serverUDP;   // UDP server in this device

//Threads
    Thread recv_thread;
    Thread send_thread; 

//Functions
void udpReceive( void );
void udpSend( void );


// WLAN security
const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

void scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;

    printf("Scan:\r\n");

    int count = wifi->scan(NULL, 0);

    /* Limit number of network arbitrary to 15 */
    count = count < 15 ? count : 15;

    ap = new WiFiAccessPoint[count];

    count = wifi->scan(ap, count);
    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\r\n", count);

    delete[] ap;
}

//Fields
char in_data[BUFF_SIZE];


int main() {
    
// Setting up WLAN
 
    printf("WiFi example\r\n\r\n");
    
    scan_demo(&esp);

    printf("\r\nConnecting...\r\n");
     int ret = esp.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\r\nConnection error\r\n");
        return -1;
    }

    // Show network address
    SocketAddress espAddress;
    printf("Success\n\n");
    printf("MAC: %s\n", esp.get_mac_address());
    esp.get_ip_address(&espAddress);
    printf("IP: %s\n", espAddress.get_ip_address());
    printf("Netmask: %s\n", esp.get_netmask());
    printf("Gateway: %s\n", esp.get_gateway());
    printf("RSSI: %d\n\n", esp.get_rssi());
    
    ThisThread::sleep_for(50ms); 
    
    serverUDP.open(&esp);
    int err = serverUDP.bind(LOCAL_PORT);
    printf("Port status is: %d\n",err);
    
    recv_thread.start(udpReceive);
    printf("Listening has been started at port number %d\n", LOCAL_PORT);
    
    send_thread.start(udpSend);
    printf("Sending out demo data to port number %d", REMOTE_PORT);
    printf(" has been started.\n");
    printf("The IP will be taken from the incoming message\n");
    
    while(1) {  
        ThisThread::sleep_for(50s);  //It doesn't matter how long the main takes
    }

}

void udpReceive()
{
    int bytes;
    while(1) {
        bytes = serverUDP.recvfrom(&clientUDP, &in_data, BUFF_SIZE);
        printf("\n");
        printf("bytes received: %d\n",bytes);
        printf("string: %s\n",in_data);
        printf("client address: %s\n", clientUDP.get_ip_address());
        printf("\n");
        }
    }


void udpSend()
{
    while(1){
        char out_data[BUFF_SIZE] = "demodata";
        clientUDP.set_port(REMOTE_PORT);
        serverUDP.sendto(clientUDP, out_data, sizeof(out_data));
        printf("Sending out: %s\n", out_data);
        printf("with %d" , sizeof(out_data));
        printf(" data bytes in UDP datagram\n");
        ThisThread::sleep_for(5s);
    }
}
