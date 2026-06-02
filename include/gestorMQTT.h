#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define MAX_REINTENTOS 10
#define TIEMPO_REINTENTO 5000
class GestorMQTT {
private:
    const char* ssid;
    const char* wifi_pass;
    const char* mqtt_broker;
    const char* mqtt_usuario;
    const char* mqtt_clave;
    const int mqtt_puerto = 8883; // Puerto seguro obligatorio

    WiFiClientSecure clienteSeguro;
    PubSubClient clienteMQTT;

    TickType_t ultimoIntentoMQTT = 0;
    const TickType_t INTERVALO_REINTENTO = pdMS_TO_TICKS(TIEMPO_REINTENTO); 

public:
    // El constructor inyecta el cliente seguro dentro del cliente MQTT
    GestorMQTT(const char* red, const char* passRed, const char* broker, const char* user, const char* passBroker)
        : clienteMQTT(clienteSeguro) 
    {
        ssid = red;
        wifi_pass = passRed;
        mqtt_broker = broker;
        mqtt_usuario = user;
        mqtt_clave = passBroker;
    }

    void iniciarWiFi() {
        int cont=0;
        Serial.print("\n[Red] Conectando a WiFi: ");
        Serial.println(ssid);
        WiFi.begin(ssid, wifi_pass);

        while (WiFi.status() != WL_CONNECTED && cont < MAX_REINTENTOS) {
            // Usamos vTaskDelay sin miedo, el setup() corre en una tarea
            vTaskDelay(pdMS_TO_TICKS(500));
            Serial.print(".");
            cont++;
        }
        if(cont< MAX_REINTENTOS)
        {
            Serial.println("\n[Red] WiFi conectado con éxito.");
        // Omitimos la validación del certificado raíz del servidor
        clienteSeguro.setInsecure();        
        clienteMQTT.setServer(mqtt_broker, mqtt_puerto);
        }
        else
        {
            Serial.println("\n[Red] Hubo un error en la conexión al WiFi");
        }
    }

    bool conectarBroker() {
        if (clienteMQTT.connected()) return true;

        Serial.print("[MQTT] Conectando a HiveMQ Cloud...");
        String clientId = "ESP32_Nonofono_" + String(random(0xffff), HEX);

        if (clienteMQTT.connect(clientId.c_str(), mqtt_usuario, mqtt_clave)) {
            Serial.println(" ¡Conexión TLS establecida!");
            
            //Nos suscribimos al canal de configuración apenas conecta
            clienteMQTT.subscribe("nonofono/config/contactos");
            Serial.println("[MQTT] Suscrito a nonofono/config/contactos");
            
            return true;
        } else {
            Serial.print(" Falló. Código de error (rc): ");
            Serial.print(clienteMQTT.state());
            Serial.println(" -> Reintentando en el próximo ciclo.");
            return false;
        }
    }

    // Debe llamarse continuamente en un loop/tarea para mantener vivo el ping con el servidor
    // Usamos el tipo de dato nativo del RTOS
    void mantenerConexion() {
        if (WiFi.status() == WL_CONNECTED) {
            if (!clienteMQTT.connected()) {
                
                TickType_t tiempoActual = xTaskGetTickCount();
                
                // Lo hace cada 5 segundos porque conectarse al broker es una acción costosa
                //y para que en caso de error le damos un respiro a la topología para que
                //conecte correctamente
                if (tiempoActual - ultimoIntentoMQTT >= INTERVALO_REINTENTO) {
                    ultimoIntentoMQTT = tiempoActual; 
                    
                    Serial.println("[MQTT] Intentando reconexión asíncrona...");
                    conectarBroker(); 
                }
            } else {
                clienteMQTT.loop();
            }
        }
    }

    void publicarAlerta(const char* topic, const char* payload) {
        if (clienteMQTT.connected()) {
            clienteMQTT.publish(topic, payload);
            Serial.println("[MQTT] Mensaje publicado con éxito.");
        } else {
            Serial.println("[MQTT] Error: Imposible publicar, el broker está desconectado.");
        }
    }
    void configurarReceptor(MQTT_CALLBACK_SIGNATURE) {
        clienteMQTT.setCallback(callback);
    }
};