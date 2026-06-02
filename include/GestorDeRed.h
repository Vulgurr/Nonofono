#include <Arduino.h>

class GestorDeRed {
public:
    // 1. Creamos el ENUM público. Estas son las "respuestas" oficiales de esta clase.
    enum EstadoRespuesta {
        REPOSO,   
        ENVIANDO, 
        EXITO,    
        ERROR     
    };

private:
    EstadoRespuesta estadoActual = REPOSO;
    unsigned long tiempoInicioPeticion = 0;
    const unsigned long TIMEOUT_WIFI = 5000;

public:
    void iniciarEnvioMensaje() {
        if (estadoActual == REPOSO || estadoActual == EXITO || estadoActual == ERROR) {
            estadoActual = ENVIANDO;
            tiempoInicioPeticion = millis();
        }
    }

    EstadoRespuesta actualizar() {
        // Si estamos enviando, calculamos qué está pasando
        if (estadoActual == ENVIANDO) {
            unsigned long tiempoTranscurrido = millis() - tiempoInicioPeticion;

            if (hayRespuestaDelServidor()) {
                estadoActual = EXITO;
            } 
            else if (tiempoTranscurrido >= TIMEOUT_WIFI) {
                estadoActual = ERROR;
            }
        }
        return estadoActual; 
    }
    void reconocerRespuesta() {
        estadoActual = REPOSO;
    }

private:
    // Funcion sin utilizar pero mockeada para una implementación futura
    bool hayRespuestaDelServidor() {
        // Simulación: 2% de probabilidad de éxito por cada ciclo
        if (random(0, 100) < 2) return true; 
        return false;
    }
};