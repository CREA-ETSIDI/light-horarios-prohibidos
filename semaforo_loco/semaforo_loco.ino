//Horarios prohibidos by Marce by CREA by tuviejosabroso
#include <Time.h>
#include <DS1307RTC.h>

// Definiciones de pines para cada placa
#ifdef ARDUINO_AVR_NANO
  // Button input
  #define MODE_SELECTOR 7 // TODO: TBR
  // OUTPUT LIGHTS
  #define RED 8
  #define BLUE 9
  #define YELLOW 10
  #define GREEN 11
#endif //!ARDUINO_AVR_NANO

#define tmDow2secs(t) (t.hour * 3600UL + t.mins * 60UL + t.secs)
typedef struct {
  uint8_t hour;
  uint8_t mins;
  uint8_t secs;
} dayOffset_t;

enum status {
  instanteUndef,
  instanteOk,
  instanteSh,
  fiestuki
};

static tmElements_t tm;
static dayOffset_t td;
static status currentStatus{instanteUndef}, prevStatus{instanteUndef};

void setup() {
  Serial.begin(9600);
  pinMode(MODE_SELECTOR, INPUT);
  pinMode(RED,    OUTPUT);
  pinMode(BLUE,   OUTPUT);
  pinMode(YELLOW, OUTPUT);
  pinMode(GREEN,  OUTPUT);

  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    Serial.println("Fallo de RTC");
}

void loop() {
  // Obtener la fecha y hora desde el chip RTC
  if (RTC.read(tm)) {
    imprimir_fecha_hora();

    td.hour = tm.Hour;
    td.mins = tm.Minute;
    td.secs = tm.Second;
    
    if (digitalRead(MODE_SELECTOR) == HIGH) {
      // Establecer cambio de modo: en fiesta cambia a indefinido y si no lo está, al fiesta
      currentStatus = (currentStatus == status::fiestuki) ? status::instanteUndef : status::fiestuki;
    }

    if (currentStatus != status::fiestuki) // Comprobamos en qué horario estamos si no es modo fiesta
      currentStatus = instanteEnHorarioSilencio(td, tm.Wday);

    if (prevStatus != currentStatus) { // Solo actualizamos si ha cambiado el estado
      switch (currentStatus)
      {
      case status::instanteOk:
        horasLibertad();
        break;
      case status::instanteSh:
        horasSilencio();
        break;
      case status::fiestuki:
        lights();
        break;
      
      default:
        break;
      }
    }
  }
  else {
    if (RTC.chipPresent()) {
      // El reloj esta detenido, es necesario ponerlo a tiempo
      Serial.print("DS1307 DETENIDO: ");
      Serial.println("EJECUTE PROGRAMA");
    }
    else {
      // No se puede comunicar con el RTC en el bus I2C, revisar las conexiones.
      Serial.print("DS1307 NO SE PUDO DETECTAR");
    }
    delay(100); // Si limpias la pantalla muy seguido se ve rara
  }
  prevStatus = currentStatus; // Almacenamos el estado anterior
}

/**
   Comprueba si el instante de tiempo pasado como argumento se encuentra en alguno de los horarios que debe haber silencio
   Cambia esta funcion para contemplar otros horarios
*/
enum status instanteEnHorarioSilencio(dayOffset_t t, uint8_t wDay) {
  static bool malMomento;
  malMomento = false;
  switch (wDay) {
    case dowTuesday:
      malMomento |= isBetween_timeIgnoreDOW(
        {17, 25, 0 }, 
        {19, 35, 0 },
        t);
      break;
    case dowThursday:
      malMomento |= isBetween_timeIgnoreDOW(
        {11, 25, 0 }, 
        {13, 35, 0 },
        t);
      break;
    case dowInvalid:
    default:
      break;
  }

  malMomento |= isBetween_timeIgnoreDOW(
    {21, 30, 0 }, 
    {8 , 30, 0 },
    t);
  return malMomento ? status::instanteSh : status::instanteOk;
}

/**
   Imprime la fecha desde la variable global "tm"
*/
void imprimir_fecha_hora(){
  // Hora
  Serial.print(tm.Hour);
  Serial.print(':');
  Serial.print(tm.Minute);
  Serial.print(':');
  Serial.print(tm.Second);
  Serial.print('\t');
  // Fecha
  Serial.print(tm.Day);
  Serial.print('/');
  Serial.print(tm.Month);
  Serial.print('/');
  Serial.println(tmYearToCalendar(tm.Year));
}

/**
   Funcion callback que activa el relevador en el pin 3 (horario prohibido)
*/
void horasSilencio()
{
  Serial.println("SHHH!");
  digitalWrite(RED, HIGH);
}
 
/**
   Funcion callback que desactiva el relevador en el pin 3 (horario de bien)
*/
void horasLibertad()
{
  Serial.println("Yay!!");
  digitalWrite(GREEN, LOW);
}

/**
   Comprueba si un dayOffset_t está entre un intervalo de horas time1 y time2
*/
bool isBetween_timeIgnoreDOW(dayOffset_t time1, dayOffset_t time2, dayOffset_t cmpre) {
  unsigned long t1Secs = tmDow2secs(time1);
  unsigned long t2Secs = tmDow2secs(time2);
  unsigned long cmpScs = tmDow2secs(cmpre);
  if (t1Secs < t2Secs) { // t1Secs is lower end of the interval
    return (t1Secs <= cmpScs && cmpScs <= t2Secs);
  }
  // else, t1Secs is upper end, so check belonging to the interval [t2Secs, t1Secs]
  return (cmpScs <= t2Secs || t1Secs <= cmpScs);
}

/** Modo fiestuqui, es posible que para que funcione haya que cambiar delays por comparación de millis
 * Sí, tocará usar millis para que no interfiera con la bellísima máquina de estados
*/
void lights() //WIP
{
  uint8_t i=0;
  uint8_t j=0;
  for (i=0; i=3; i++){
    switch (i) {
      case 0:
        j= RED;
      break;
      case 1:
        j= BLUE;
      break;
      case 2:
        j= YELLOW;
      break;
      case 3:
        j= GREEN;
      break;
    }
    digitalWrite(j, HIGH);
    digitalWrite(j, LOW);
  }
  for (i=3; i=0; i--){
    switch (i) {
      case 0:
        j= RED;
      break;
      case 1:
        j= BLUE;
      break;
      case 2:
        j= YELLOW;
      break;
      case 3:
        j= GREEN;
      break;
    }
    digitalWrite(j, HIGH);
    digitalWrite(j, LOW);
  }
}
