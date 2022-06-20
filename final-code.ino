// All variables
float corrienteNominal;
float voltajeNominal;
float mulCorrienteNominalTmpoInv;
float mulCorrienteNominalTmpoInst;
int tmpoSobreCorrInst;
float arranqueBajoVoltajeTmpoCte;
float arranqueBajoVoltajeTmpoInst;
float arranqueSobreVoltajeTmpoCte;
float arranqueSobreVoltajeTmpoInst;
int tmpoInstVoltaje;
int tmpoCteVoltaje;
float mulCorrienteArranqueRotorTranc;
float tmpoRotTrancado;
int numeroDeArranques;
int tmpoDeNumeroArranques;
float subMultTrabVacio;
int tmpoTrabVacio;
float factorDeServicio;
float tiempoInterrupcionSobreCorriente;
int contadorMaximoSobreCorriente;
float voltaje;
float relacionDeCorriente;
int tiempoPresionadoDeArranqueAnterior;
int tiempoDeArranqueInicial;
int numeroDeArranquesPermitidos;
int tiempoEsperaDeInicio;
int tiempoAnterior;
int tiempoActual;
int voltajeSuperiorFallaCritica;
int voltajeSuperiorFallaTiempoConstante;
int voltajeInferiorFallaCritica;
int voltajeInferiorFallaTiempoConstante;
int maximoVoltajeARecibir;
int tiempoInicioFallaTiempoConstante;
int tiempoActualFallaTiempoConstante;
int tiempoEnFallaConstante;
int_fast64_t tiempoMultiplesArranques;
boolean contadorMaximoAlcanzado;
boolean timer3Activo;
boolean estadoEnfriamiento;
boolean proteccionDeVoltajeActiva;
boolean modoEnfriamientoActivo;
boolean apagadoPorCondicionExtrema;
boolean motorIniciado;
boolean proteccionDeVoltajeInstantaneaActiva;
// StartPinVariables
int pinDeVoltaje = A0;
int pinDeCorriente = A8;
int pinSalidaDeMotor = 10;
int pinIndicadorVoltaje = 22;
int pinIndicadorSobreCorriente = 24;
int pinIndicadorMaximoContador = 26;
int pinIndicadorEnfriamiento = 28;
int pinInterrupcionDeArranque = 3;

// Setup de variables valor defecto
//Ajustar tiempo de marcha en vacio
void SetupVariables()
{
  // Voltaje
  tmpoInstVoltaje = 1000;
  voltajeSuperiorFallaCritica = 140;
  voltajeSuperiorFallaTiempoConstante = 130;
  voltajeInferiorFallaCritica = 70;
  voltajeInferiorFallaTiempoConstante = 90;
  maximoVoltajeARecibir = 150;
  proteccionDeVoltajeInstantaneaActiva = false;
  tiempoEnFallaConstante = 20;

  // Corriente
  mulCorrienteNominalTmpoInv = 1.15;
  corrienteNominal = 2;
  contadorMaximoSobreCorriente = 0;
  estadoEnfriamiento = false;
  contadorMaximoAlcanzado = false;
  proteccionDeVoltajeActiva = false;
  timer3Activo = false;
  tmpoTrabVacio = 50;
  apagadoPorCondicionExtrema = false;

  //Arranques
  numeroDeArranques = 1;
  numeroDeArranquesPermitidos = 3;
  tiempoMultiplesArranques = 60000;
  tiempoEsperaDeInicio = 4000;
  tiempoPresionadoDeArranqueAnterior = millis();
  motorIniciado = false;
  tiempoAnterior = millis();
}

// Functions here

// Modulos para protección de voltaje
float leerVoltaje()
{
  float lecturaEnVoltios = analogRead(pinDeVoltaje) * 5.0 / 1023.0;
  return lecturaEnVoltios * 150 / 5;
}

boolean voltajeNoEsCorrecto()
{
  return voltaje < voltajeInferiorFallaTiempoConstante || voltaje > voltajeSuperiorFallaTiempoConstante;
}

boolean desconexionInstantanea(){
  return voltaje < voltajeInferiorFallaCritica || voltaje > voltajeSuperiorFallaCritica;
}

//Arranca sin importar la falla desde 0
//Lo sacamos de ese modo apenas se acabe el tiempo
//Hay trabajo normal, falla y desconexión instantanea
//Pasar el voltaje recibido a voltios

void proteccionDeVoltaje()
{
  if (desconexionInstantanea() && !proteccionDeVoltajeInstantaneaActiva)
  {
    digitalWrite(pinIndicadorVoltaje, HIGH);
    proteccionDeVoltajeInstantaneaActiva = true;
    int tiempoInicial = millis();
    while (millis() - tiempoInicial <= tmpoInstVoltaje)
    {
      millis();
    }
    if (voltajeNoEsCorrecto())
    {
      motorIniciado = false;
      digitalWrite(pinSalidaDeMotor, LOW);
      return;
    }
  }
  if (voltajeNoEsCorrecto() && !proteccionDeVoltajeActiva)
  {
    digitalWrite(pinIndicadorVoltaje, HIGH);
    proteccionDeVoltajeActiva = true;
    tiempoInicioFallaTiempoConstante = millis()/1000;
    return;
  }
  if (proteccionDeVoltajeActiva)
  {
    tiempoActualFallaTiempoConstante = millis()/1000;
    if (!voltajeNoEsCorrecto())
    {
      digitalWrite(pinIndicadorVoltaje, LOW);
      proteccionDeVoltajeActiva = false;
      return;
    }
    
    if (voltajeNoEsCorrecto() && tiempoActualFallaTiempoConstante - tiempoInicioFallaTiempoConstante >= tiempoEnFallaConstante) 
    {
      motorIniciado = false;
      digitalWrite(pinSalidaDeMotor, LOW);
      return;
    }
  } 
  if (proteccionDeVoltajeInstantaneaActiva)
  {
    digitalWrite(pinIndicadorVoltaje, LOW);
    proteccionDeVoltajeInstantaneaActiva = false;
  }
}

// Modulos para protección de corriente
float leerRelacionDeCorriente()
{
  float current = analogRead(pinDeCorriente) * 30 / 1023.0;
  return current / corrienteNominal;
}

// Arrancamos con sobreCorriente
boolean haySobreCorriente()
{
  return relacionDeCorriente > mulCorrienteNominalTmpoInv;
}

void seleccionarEcuacion()
{
  if (relacionDeCorriente >= 1 && relacionDeCorriente <= 1.2)
  {
    tiempoInterrupcionSobreCorriente = -(relacionDeCorriente - 1.2) * 100000 + (relacionDeCorriente - 1.1) * 8500;
    return;
  }
  if (relacionDeCorriente > 1.2 && relacionDeCorriente <= 1.3)
  {
    tiempoInterrupcionSobreCorriente = -(relacionDeCorriente - 1.3) * 8500 + (relacionDeCorriente - 1.2) * 2800;
    return;
  }
  if (relacionDeCorriente > 1.3 && relacionDeCorriente <= 1.5)
  {
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente - 1.4) * (relacionDeCorriente - 1.5) * 14000 - (relacionDeCorriente - 1.3) * (relacionDeCorriente - 1 - 5) * 17000 + (relacionDeCorriente - 1.3) * (relacionDeCorriente - 1.4) * 6500;
    return;
  }
  if (relacionDeCorriente > 1.5 && relacionDeCorriente <= 2)
  {
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente - 1.8) * (relacionDeCorriente - 2) * 866.6667 - (relacionDeCorriente - 1.5) * (relacionDeCorriente - 2) * 1250 + (relacionDeCorriente - 1.5) * (relacionDeCorriente - 1.8) * 600;
    return;
  }
  if (relacionDeCorriente > 2 && relacionDeCorriente <= 4)
  {
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente - 3) * (relacionDeCorriente - 4) * 30 - (relacionDeCorriente - 2) * (relacionDeCorriente - 4) * 24 + (relacionDeCorriente - 2) * (relacionDeCorriente - 3) * 7;
    return;
  }
  if (relacionDeCorriente > 4 && relacionDeCorriente <= 7)
  {
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente - 5) * (relacionDeCorriente - 7) * 4.6666 - (relacionDeCorriente - 4) * (relacionDeCorriente - 7) * 5 + (relacionDeCorriente - 4) * (relacionDeCorriente - 5) * 1.0666;
    return;
  }
  if (relacionDeCorriente > 7 && relacionDeCorriente <= 12)
  {
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente - 10) * (relacionDeCorriente - 12) * 0.4266 - (relacionDeCorriente - 7) * (relacionDeCorriente - 12) * 0.6833 + (relacionDeCorriente - 7) * (relacionDeCorriente - 10) * 0.32;
    return;
  }
  tiempoInterrupcionSobreCorriente = 3.2;
  return;
}

void iniciarInterrupcionTimer3()
{
  cli();
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= B00000100;
  TIMSK3 |= B00000010;
  OCR3A = (6.25 * tiempoInterrupcionSobreCorriente) - 1;
  sei();
  timer3Activo = true;
}

void apagarInterrupcionTimer3()
{
  TIMSK3 &= ~(1 << OCIE1A);
  timer3Activo = false;
}

void guardianMaximoValorContador()
{
  if (contadorMaximoSobreCorriente >= 10000)
  {
    contadorMaximoAlcanzado = true;
    digitalWrite(pinIndicadorMaximoContador, contadorMaximoAlcanzado);
    digitalWrite(pinSalidaDeMotor, LOW);
    apagarInterrupcionTimer3();
    return;
  }
}

void actualizarTimerYContador()
{
  if (contadorMaximoSobreCorriente < 10000)
  {
    contadorMaximoSobreCorriente++;
    seleccionarEcuacion();
    // Funcion para actualizar los valores del timer
    iniciarInterrupcionTimer3();
    guardianMaximoValorContador();
  }
}

ISR(TIMER3_COMPA_vect)
{ // Interrupción usada para validar corriente según el tiempo dado
  actualizarTimerYContador();
  TCNT3 = 0;
}

// Modo enfriamiento

void guardianValorCeroContador()
{
  if (contadorMaximoSobreCorriente == 0)
  {
    contadorMaximoAlcanzado = false;
    digitalWrite(pinIndicadorMaximoContador, contadorMaximoAlcanzado);
    digitalWrite(pinIndicadorEnfriamiento, false);
    apagarInterrupcionTimer5();

    return;
  }
}

void modoEnfriamiento()
{
  if (contadorMaximoSobreCorriente > 0)
  {
    contadorMaximoSobreCorriente--;
    estadoEnfriamiento = !estadoEnfriamiento;
    digitalWrite(pinIndicadorEnfriamiento, estadoEnfriamiento);
  }
  guardianValorCeroContador();
}

void revisarSobreCorriente()
{
  if (haySobreCorriente() && !timer3Activo && !apagadoPorCondicionExtrema)
  {
    digitalWrite(pinIndicadorSobreCorriente, HIGH);
    actualizarTimerYContador();
    estadoEnfriamiento = false;
    digitalWrite(pinIndicadorEnfriamiento, estadoEnfriamiento);
    apagarInterrupcionTimer5();
    iniciarInterrupcionTimer3();
    return;
  }
  if (!haySobreCorriente() && !hayMarchaEnVacio())
  {
    
    digitalWrite(pinIndicadorSobreCorriente, LOW);
    // Si no hay sobreCorriente desactivamos timer3
    if (timer3Activo && !apagadoPorCondicionExtrema)
    {
      apagarInterrupcionTimer3();
      iniciarTimerDeUnSegundo();
    }
    // Activamos otro timer para enfriamiento a razón de 1hz
    return;
  }
}

void iniciarTimerDeUnSegundo()
{
  cli();
  TCCR5A = 0;
  TCCR5B = 0;
  TCCR5B |= B00000100;
  TIMSK5 |= B00000010;
  OCR5A = (6.25 * 62499) - 1;
  sei();
  modoEnfriamientoActivo = true;
}

void apagarInterrupcionTimer5()
{
  TIMSK5 &= ~(1 << OCIE5A);
  modoEnfriamientoActivo = false;
}

ISR(TIMER5_COMPA_vect)
{ // Interrupción encargada de enfriar a razon de 1 hz
  TCNT5 = 0;
  modoEnfriamiento();
}

// Protección por corriente instantaneas

boolean hayMarchaEnVacio(){
  if (relacionDeCorriente < 0.3){
    int tiempoInicial = millis();
    while (millis() - tiempoInicial <= tmpoTrabVacio)
    {
      millis();
    }
    if (relacionDeCorriente < 0.3)
    {
      return true;
    }
  }
return false;
}

boolean haySobreCorrienteInstantanea(){
  if (relacionDeCorriente > 12)
  {
    int tiempoInicial = millis();
    while (millis() - tiempoInicial <= tmpoTrabVacio)
    {
      millis();
    }
    if (relacionDeCorriente > 12)
    {
      return true;
    }
  }
  return false;
}

void protegerPorCorrientesExtremas(){
  if (haySobreCorrienteInstantanea() || hayMarchaEnVacio())
  {
    apagarInterrupcionTimer3();
    apagarInterrupcionTimer5();
    contadorMaximoSobreCorriente = 0;
    estadoEnfriamiento = false;
    apagadoPorCondicionExtrema = true;
    digitalWrite(pinIndicadorEnfriamiento, estadoEnfriamiento);
    digitalWrite(pinIndicadorSobreCorriente, HIGH);
    digitalWrite(pinSalidaDeMotor, false);
  }
  
}

void protectorDeCorriente(){
  revisarSobreCorriente();
  protegerPorCorrientesExtremas();
}

// Lector de voltaje y corriente

void setupTimerInterruption()
{
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= B00000100;
  TIMSK1 |= B00000010;
  OCR1A = (6.25 * 5) - 1;
  sei();
}

ISR(TIMER1_COMPA_vect){ // Interrupción encargada de solo leer voltaje y corriente
  TCNT1 = 0;
  relacionDeCorriente = leerRelacionDeCorriente();
  voltaje = leerVoltaje();
}

//Arranque

  //Antirrebote
boolean proteccionAntirebote(){
  if (tiempoPresionadoDeArranqueAnterior - millis() > 300)
  {
    tiempoPresionadoDeArranqueAnterior = millis();
    return false;
  }
  return true;
}

 void setupTimer2Interruption(){
  cli();
  TCCR2A = 0;
  TCCR2B = 0;
  TCCR2B |= B00000101;
  TIMSK2 |= B00000010;
  OCR2A = 255;
  sei();
 }

ISR(TIMER2_COMPA_vect){ //Interrupción encargada de actualizar la función miliis (Usada para tiempos fijos)
  TCNT2  = 0;
  tiempoActual = millis();
}

boolean proteccionArranqueRepetido(){
  if (numeroDeArranques == 1)
  {
    tiempoDeArranqueInicial = millis();
  }
  if(numeroDeArranques > numeroDeArranquesPermitidos && millis() - tiempoDeArranqueInicial < tiempoMultiplesArranques){
    return true;
  }
  return false;
}

boolean corrienteDeArranqueNoEsCorrecto(){
  return relacionDeCorriente > mulCorrienteNominalTmpoInv || relacionDeCorriente < 0.3;
}

void logicaDeArranque(){
    digitalWrite(pinSalidaDeMotor, true);
    tiempoAnterior = millis();
    tiempoActual = millis();
    setupTimer2Interruption();
    while (tiempoActual - tiempoAnterior < tiempoEsperaDeInicio)
    {
      tiempoActual = millis();
    }
    if (voltajeNoEsCorrecto() || corrienteDeArranqueNoEsCorrecto())
    {
      motorIniciado = false;
      digitalWrite(pinSalidaDeMotor, false);
      apagadoPorCondicionExtrema = true;
      return;
    }
    motorIniciado = true;
    numeroDeArranques++;
}

void funcionDeArranque(){
  delay(10);
  Serial.println(digitalRead(pinInterrupcionDeArranque));
  if(digitalRead(pinInterrupcionDeArranque) && !apagadoPorCondicionExtrema && contadorMaximoSobreCorriente <= 0){
    if (!proteccionAntirebote() && !proteccionArranqueRepetido())
    {
      logicaDeArranque();
    }
  return;
  }
  apagarInterrupcionTimer3();
  motorIniciado = false;
  digitalWrite(pinSalidaDeMotor, false);
}

void setup()
{
  Serial.begin(9600);
  SetupVariables();
  setupTimerInterruption();
  pinMode(pinSalidaDeMotor, OUTPUT);
  pinMode(pinIndicadorVoltaje, OUTPUT);
  digitalWrite(pinSalidaDeMotor, false);
  digitalWrite(pinIndicadorVoltaje, false);
  attachInterrupt(digitalPinToInterrupt(pinInterrupcionDeArranque), funcionDeArranque, CHANGE);

}

void loop()
{
  if (motorIniciado)
  {
    protectorDeCorriente();
    proteccionDeVoltaje();
  }
  
}