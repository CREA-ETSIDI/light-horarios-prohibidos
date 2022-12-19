//Horarios prohibidos by Marce by CREA by tuviejosabroso
#include <iso646.h>
#include "DS1307RTC.h"

// #define LOG

// Todo sea por que las extensiones de VSCode funcionen
#include <Arduino.h>
#define ARDUINO_AVR_NANO

// Flags para las luces que se van a encender por posición
#define LIGHT_FLAG_1 0x01
#define LIGHT_FLAG_2 0x02
#define LIGHT_FLAG_3 0x04
#define LIGHT_FLAG_4 0x08

// Definiciones de pines para cada placa
#ifdef ARDUINO_AVR_NANO
  // Button change-mode input
  #define MODE_SELECTOR 7 // TODO: TBR
  // OUTPUT LIGHTS
  #define LIGHT_PIN_1 9
  #define LIGHT_PIN_2 10
  #define LIGHT_PIN_3 11
  #define LIGHT_PIN_4 12
#endif //!ARDUINO_AVR_NANO

/* Some logging utilities */
// More at the end of file
#ifdef LOG
    #define serialLog(msg) Serial.print(msg)
    #define serialLogLn(msg) Serial.println(msg)
#else
    #define serialLog(msg)
    #define serialLogLn(msg)
#endif

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
  janitorApproaching,
  emptyness,
  fiestuki
};

static tmElements_t tm;
static dayOffset_t td;
static status currentStatus{instanteUndef}, prevStatus{instanteUndef};

void setup() {
#ifdef LOG
  Serial.begin(9600);
#endif
  pinMode(MODE_SELECTOR, INPUT);
  pinMode(LIGHT_PIN_1, OUTPUT);
  pinMode(LIGHT_PIN_2, OUTPUT);
  pinMode(LIGHT_PIN_3, OUTPUT);
  pinMode(LIGHT_PIN_4, OUTPUT);

  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet)
    serialLogLn("Fallo de RTC");
  serialLogLn("Arduino Online");
}

void loop() {
  // Obtener la fecha y hora desde el chip RTC
  if (RTC.read(tm)) {
#ifdef LOG
    imprimir_fecha_hora();
#endif

    td.hour = tm.Hour;
    td.mins = tm.Minute;
    td.secs = tm.Second;
    
    if (digitalRead(MODE_SELECTOR) == HIGH) {
      // Establecer cambio de modo: en fiesta cambia a indefinido y si no lo está, al fiesta
      currentStatus = (currentStatus == status::fiestuki) ? status::instanteUndef : status::fiestuki;
      delay(150);
    }

    if (currentStatus != status::fiestuki) // Comprobamos en qué horario estamos si no es modo fiesta
      currentStatus = obtenerInstante(td, tm.Wday);

    switch (currentStatus)
    {
      case status::instanteOk:
      case status::instanteSh:
      case status::janitorApproaching:
      case status::emptyness:
        if (prevStatus != currentStatus) { // Solo actualizamos si ha cambiado el estado
          switch_lights(0x0F);
          delay(5*1000);
          if (currentStatus == status::instanteOk)
            switch_lights(LIGHT_FLAG_1);
          else if (currentStatus == status::instanteSh)
            switch_lights(LIGHT_FLAG_4);
          else if (currentStatus == status::janitorApproaching)
            switch_lights(LIGHT_FLAG_2);
          else if (currentStatus == status::emptyness)
            switch_lights(0x00);
        }
        break;
      case status::fiestuki:
        // serialLogLn("Estamos en fiesta");
        modo_fiestuki();
        break;

      default:
        break;
    }
    customLog(currentStatus);
  }
  else {
    if (RTC.chipPresent()) {
      // El reloj esta detenido, es necesario ponerlo a tiempo
      serialLogLn("DS1307 DETENIDO: EJECUTE PROGRAMA DE CARGA");
    }
    else {
      // No se puede comunicar con el RTC en el bus I2C, revisar las conexiones.
      serialLogLn("DS1307 NO SE PUDO DETECTAR");
    }
    delay(250); // Si limpias la pantalla muy seguido se ve rara
  }
  prevStatus = currentStatus; // Almacenamos el estado anterior
}

/**
   Comprueba si el instante de tiempo pasado como argumento se encuentra en alguno de los horarios que debe haber silencio
   Cambia esta funcion para contemplar otros horarios
*/
enum status obtenerInstante(dayOffset_t t, timeDayOfWeek_t wDay) {
  bool momentoSilencio{false}, momentoIrSaliendo{false}, momentoVacio{false};
  switch (wDay) {
    case dowThursday:
      momentoSilencio |= isBetween_timeIgnoreDOW(
        {11, 25, 0 },
        {13, 35, 0 },
        t);
      momentoSilencio |= isBetween_timeIgnoreDOW(
        {15, 25, 0 },
        {19, 35, 0 },
        t);
      break;
    case dowFriday:
      momentoSilencio |= isBetween_timeIgnoreDOW(
        {11, 25, 0 },
        {13, 35, 0 },
        t);
      break;
    case dowInvalid:
    default:
      break;
  }
  // Fuera de hora todos los días
  momentoSilencio |= isBetween_timeIgnoreDOW(
    {21, 30, 0 },
    {8 , 30, 0 },
    t);

  // Ojo que vienen los conserjes a darnos una paliza gitana
  momentoIrSaliendo |= isBetween_timeIgnoreDOW(
    {21, 15, 0 },
    {21, 30, 0 },
    t);
  
  // No es necesario encender nada ya que no debería haber nadie (repito: NADIE) en la sala
  momentoVacio |= isBetween_timeIgnoreDOW(
    {21, 31, 0 },
    {8 , 30, 0},
    t);
  serialLog("momento vacío  I: ");
  serialLogLn(momentoVacio);

  momentoVacio |= (wDay == dowSaturday) or (wDay == dowSunday);
  serialLog("momento vacío II: ");
  serialLogLn(momentoVacio);

  if(momentoVacio)
    return status::emptyness;
  if(momentoIrSaliendo)
    return status::janitorApproaching;
  return momentoSilencio ? status::instanteSh : status::instanteOk;
}

/**
   Imprime la fecha desde la variable global "tm"
*/
void imprimir_fecha_hora(){
  // Hora
  serialLog(tm.Hour);
  serialLog(':');
  if(tm.Minute < 9)
    serialLog('0');
  serialLog(tm.Minute);
  serialLog(':');
  if(tm.Second < 9)
    serialLog('0');
  serialLog(tm.Second);
  serialLog('\t');
  // Dia
  serialLog(tm.Wday);
  serialLog('\t');
  // Fecha
  if(tm.Day < 9)
    serialLog('0');
  serialLog(tm.Day);
  serialLog('/');
  serialLog(tm.Month);
  serialLog('/');
  serialLogLn(tmYearToCalendar(tm.Year));
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
void modo_fiestuki() //WIP
{
  uint8_t i = 0;
  uint8_t lights_mask = 0;
  static unsigned long pasttime;
  static unsigned long cycletime;
  static int ciclo;
  serialLogLn(ciclo);
  delay(100);
  switch (ciclo)
  {
  case 0:
    cycletime = pasttime = millis();
    ciclo++;
    break;
  case 1:
  case 3:
  case 5:
    i = static_cast<unsigned long> (millis()-cycletime)/500;
    lights_mask = 0x01 << i;
    if(i > 3){
      cycletime = millis();
      ciclo++;
    }
    switch_lights(lights_mask);
    break;
  case 2:
  case 4:
  case 6:
    i = static_cast<unsigned long> (millis()-cycletime)/500;
    lights_mask = 0x08 >> i;
    if(i > 3){
      cycletime = millis();
      ciclo++;
    }
    switch_lights(lights_mask);
    break;
  case 7:
    i = static_cast<unsigned long> (millis()-cycletime)/500;
    if(i > 20){
      cycletime = millis();
      ciclo++;
    }
    if(i%2==0){
      lights_mask = 0x05;
    }
    else{
      lights_mask = 0x0A;
    }
    switch_lights(lights_mask);
    break;
  default:
    ciclo = 0;
  }
  serialLogLn(i);
  // delay(250);
}

/**
 * @brief Turns on and off lights depending on a binary mask
 * 
 * @param lights_mask 8-bit mask to set the output to each bit
 */
void switch_lights(uint8_t lights_mask){
  digitalWrite(LIGHT_PIN_1, LIGHT_FLAG_1&lights_mask);
  digitalWrite(LIGHT_PIN_2, LIGHT_FLAG_2&lights_mask);
  digitalWrite(LIGHT_PIN_3, LIGHT_FLAG_3&lights_mask);
  digitalWrite(LIGHT_PIN_4, LIGHT_FLAG_4&lights_mask);
}

/* Reporte serial */
#ifdef LOG
void customLog(status st) {
  switch (st)
  {
  case status::instanteUndef:
    serialLogLn("status::instanteUndef");
    break;
  case status::instanteOk:
    serialLogLn("status::instanteOk");
    break;
  case status::instanteSh:
    serialLogLn("status::instanteSh");
    break;
  case status::fiestuki:
    serialLogLn("status::fiestuki");
    break;
  case status::janitorApproaching:
    serialLogLn("status::janitorApproaching");
    break;
  case status::emptyness:
    serialLogLn("status::emptyness");
    break;
  default:
    serialLogLn("Switch default. Status not found");
    break;
  }
}
#else
void customLog(status st){};
#endif
