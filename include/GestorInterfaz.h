#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// ==========================================
// GESTOR DE INTERFAZ (LCD y Avisos)
// ==========================================
class GestorDeInterfaz {
public:
    // Configuración inicial
    void iniciar() {
        lcd.init();
        //lcd.backlight();
    }


    void mostrarPantallaInit() {
        lcd.clear();
        lcd.setCursor(5, 0); lcd.print("SISTEMA DE");
        lcd.setCursor(5, 1); lcd.print("ASISTENCIA");
        lcd.setCursor(6, 2); lcd.print("NONOFONO");

    }


    void mostrarPantallaReposo() {
        lcd.clear();
        lcd.setCursor(1, 0); lcd.print("BIENVENIDO CARLOS");
        lcd.setCursor(0, 1); lcd.print("PRESIONE ARRIBA ^");
        lcd.setCursor(0, 2); lcd.print("O ABAJO v PARA");
        lcd.setCursor(0, 3); lcd.print("ENVIAR MENSAJES");
    }
    int obtenerCantidadEfectiva(String contactos[10]) {
    int cantidad = 0;
    for (int i = 0; i < 10; i++) {
        if (contactos[i].length() > 0) {
            cantidad++;
        } else {
            break; // Al primer string vacío, terminamos de contar
        }
    }
    return cantidad;
}
void mostrarProcesandoAudio() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   PROCESANDO   ");
    lcd.setCursor(0, 1);
    lcd.print(" AUDIO, ESPERE. ");
    
    // Si tu pantalla es de 20x4, puedes agregar una animación simple o más texto
    // lcd.setCursor(0, 2);
    // lcd.print(" Guardando en SD...");
}

    // Recibe un array de contactos y el índice actual para dibujar el cursor "<-"
void mostrarNavegandoContactos(String contactos[10], int indiceSeleccionado) {
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("SELECCIONE CONTACTO:");

    int cantidadEfectiva = obtenerCantidadEfectiva(contactos);

    if (cantidadEfectiva == 0) {
        lcd.setCursor(0, 1);
        lcd.print("No hay contactos");
        return;
    }

    if (indiceSeleccionado>cantidadEfectiva)
        indiceSeleccionado=cantidadEfectiva;
    int indiceTop = 0;
    if (indiceSeleccionado >= 3) {
        indiceTop = indiceSeleccionado - 2;
    }

    for (int i = 0; i < 3; i++) {
        int indiceActual = indiceTop + i;
        
        if (indiceActual < cantidadEfectiva) { 
            lcd.setCursor(0, i + 1);
            lcd.print(contactos[indiceActual]);
            
            if (indiceActual == indiceSeleccionado) {
                lcd.print(" <-"); 
            }
        }
    }
}

    void mostrarConfirmarContacto(String contactoSeleccionado) {
        lcd.clear();
        lcd.setCursor(0, 1); lcd.print("CONTACTO ELEGIDO:");
        lcd.setCursor(0, 2); lcd.print(contactoSeleccionado);
    }


    void mostrarGrabando(String contactoSeleccionado) {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("* GRABANDO... *");
        lcd.setCursor(0, 1); lcd.print("PARA:");
        lcd.setCursor(0, 2); lcd.print(contactoSeleccionado);
    }


    void mostrarConfirmarAudio() {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print(char(168)); // Símbolo '¿' en ASCII extendido
        lcd.print("DESEA CONFIRMAR");
        lcd.setCursor(0, 1); lcd.print("EL MENSAJE?");
        lcd.setCursor(0, 2); lcd.print("   CONFIRMAR");
        lcd.setCursor(0, 3); lcd.print("   CANCELAR");
    }

void mostrarMensajesPredefinidos(String contacto, String mensajes[3], int indiceSeleccionado) {
    lcd.clear();
    
    lcd.setCursor(0, 0); 
    lcd.print("ENVIAR A ");
    lcd.print(contacto);
    lcd.print(":");
    
    //Aseguramos que el índice nunca se salga del rango 0-2
    int indiceSeguro = constrain(indiceSeleccionado, 0, 2);


    for (int i = 0; i < 3; i++) {
        lcd.setCursor(0, i + 1);
        
        lcd.print(mensajes[i]);
        if (i == indiceSeguro) {
            lcd.print("<-"); 
        } else {
            lcd.print("  "); // Mantenemos la misma cantidad de espacios en blanco
        }
        
    }
}

    // EMERGENCIA (Prioridad absoluta)
    void mostrarEmergencia() {
        lcd.clear();
        lcd.setCursor(0, 0); lcd.print("!!! EMERGENCIA !!!");
        lcd.setCursor(0, 1); lcd.print("LLAMANDO A");
        lcd.setCursor(0, 2); lcd.print("EMERGENCIAS Y A");
        lcd.setCursor(0, 3); lcd.print("SUS FAMILIARES");
    }
    

    void limpiarPantalla() {
        lcd.clear();
    }
    void mostrarEnviando() {
        lcd.clear();
        lcd.setCursor(2, 0); 
        lcd.print("ENVIANDO MENSAJE");
        
        lcd.setCursor(0, 2);
        lcd.print(" POR FAVOR, ESPERE");
        
        lcd.setCursor(8, 3);
        lcd.print("...");
    }

    void mostrarExito() {
        lcd.clear();
        
        lcd.setCursor(2, 1);
        lcd.print("MENSAJE ENVIADO");
        
        lcd.setCursor(4, 2);
        lcd.print("CON EXITO!");
    }

    void mostrarError() {
        lcd.clear();
        
        lcd.setCursor(2, 0);
        lcd.print("!!! ATENCION !!!");
        
        lcd.setCursor(0, 1);
        lcd.print("NO SE PUDO ENVIAR");
        
        lcd.setCursor(0, 2);
        lcd.print("EL MENSAJE.");
        
        lcd.setCursor(0, 3);
        lcd.print("INTENTE DE NUEVO.");
    }
};

// Instancia global del gestor para usar en el loop()
GestorDeInterfaz gestorUI;