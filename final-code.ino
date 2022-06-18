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
boolean contadorMaximoAlcanzado;
boolean timer3Activo;
boolean estadoEnfriamiento;
// StartPinVariables
int pinDeVoltaje = A0;
int pinDeCorriente = A8;
int pinSalidaDeMotor = 10;
int pinIndicadorVoltaje = 22;
int pinIndicadorSobreCorriente = 24;
int pinIndicadorMaximoContador = 26;
int pinIndicadorEnfriamiento = 28;

//Setup de variables valor defecto
void SetupVariables(){
  //Voltaje
  tmpoInstVoltaje = 1000;

  //Corriente
  mulCorrienteNominalTmpoInv = 1.15;
  corrienteNominal = 2;
  contadorMaximoSobreCorriente = 0;
  estadoEnfriamiento = false;
  contadorMaximoAlcanzado = false;
  timer3Activo = false;
}

// Functions here

  //Modulos para protección de voltaje
float leerVoltaje(){
  return analogRead(pinDeVoltaje) * 5.0 / 1023.0;
}

boolean voltajeNoEsCorrecto(float voltaje){
  return voltaje <= 3.6 || voltaje >= 4.4;
}

void proteccionDeVoltaje(){
  float voltaje = leerVoltaje();
  if (voltajeNoEsCorrecto){
    digitalWrite(pinIndicadorVoltaje, HIGH);
    int tiempoInicial = millis();
    while (millis() - tiempoInicial <= tmpoInstVoltaje){
      millis();
    }
    if (voltajeNoEsCorrecto){
      digitalWrite(pinSalidaDeMotor, LOW);
      return;
    }
    digitalWrite(pinIndicadorVoltaje, LOW);
  }
}

// Modulos para protección de corriente
float leerRelacionDeCorriente(){
  float current = analogRead(pinDeCorriente) * 30 / 1023.0;
  return current / corrienteNominal;
}

  //Arrancamos con sobreCorriente
boolean haySobreCorriente(float relacionDeCorriente){
  return relacionDeCorriente > mulCorrienteNominalTmpoInv;
}

void seleccionarEcuacion(float relacionDeCorriente){
  if(relacionDeCorriente >= 1 && relacionDeCorriente <= 1.2){
    tiempoInterrupcionSobreCorriente = -(relacionDeCorriente -1.2) * 100000 + (relacionDeCorriente - 1.1) * 8500;
    return;
  }
  if(relacionDeCorriente > 1.2 && relacionDeCorriente <= 1.3){
    tiempoInterrupcionSobreCorriente= -(relacionDeCorriente -1.3) * 8500 + (relacionDeCorriente -1.2) * 2800;
    return;
  }
  if(relacionDeCorriente > 1.3 && relacionDeCorriente <= 1.5){
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente -1.4)*(relacionDeCorriente-1.5)*14000-(relacionDeCorriente-1.3)*(relacionDeCorriente-1-5)*17000 + (relacionDeCorriente -1.3)*(relacionDeCorriente-1.4)*6500;
    return;
  }
  if(relacionDeCorriente > 1.5 && relacionDeCorriente <= 2){
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente-1.8)*(relacionDeCorriente-2)*866.6667-(relacionDeCorriente-1.5)*(relacionDeCorriente-2)*1250+(relacionDeCorriente-1.5)*(relacionDeCorriente -1.8)*600;
    return;
  }
  if(relacionDeCorriente > 2 && relacionDeCorriente <= 4){
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente-3)*(relacionDeCorriente-4)*30-(relacionDeCorriente-2)*(relacionDeCorriente-4)*24+(relacionDeCorriente-2)*(relacionDeCorriente-3)*7;
    return;
  }
  if(relacionDeCorriente > 4 && relacionDeCorriente <= 7){
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente-5)*(relacionDeCorriente-7)*4.6666-(relacionDeCorriente-4)*(relacionDeCorriente-7)*5+(relacionDeCorriente-4)*(relacionDeCorriente-5)*1.0666;
    return;
  }
  if(relacionDeCorriente > 7 && relacionDeCorriente <= 12){
    tiempoInterrupcionSobreCorriente = (relacionDeCorriente-10)*(relacionDeCorriente-12)*0.4266-(relacionDeCorriente-7)*(relacionDeCorriente-12)*0.6833+(relacionDeCorriente-7)*(relacionDeCorriente-10)*0.32;
    return;
  }
    tiempoInterrupcionSobreCorriente = 3.2;
    return;
}

/* void iniciarInterrupcionTimer3(){
  cli();
  TCCR3A = 0;
  TCCR3B = 0;
  TCCR3B |= B00000100;
  TIMSK3 |= B00000010;
  OCR3A = (6.25 * tiempoInterrupcionSobreCorriente) - 1;
  sei();
 } 
 
 void apagarInterrupcionTimer3(){
  TIMSK3 &= ~(1 << OCIE1A);
  timer3Activo = false;
 }
 
 */

void guardianMaximoValorContador(){
  if (contadorMaximoSobreCorriente == 10000)
  {
    digitalWrite(contadorMaximoAlcanzado, HIGH);
    digitalWrite(pinSalidaDeMotor, LOW);
    //apagarInterrupcionTimer3();
    return;
  }
  
}

void actualizarTimerYContador(float relacionDeCorriente){
    contadorMaximoSobreCorriente++;
    seleccionarEcuacion(relacionDeCorriente);
    //Funcion para actualizar los valores del timer
    //iniciarInterrupcionTimer3();
    guardianMaximoValorContador();
}

ISR(TIMER3_COMPA_vect){ //Interrupción usada para validar corriente según el tiempo dado
  float relacionDeCorriente = leerRelacionDeCorriente();
  actualizarTimerYContador(relacionDeCorriente);
  //TCNT3  = 0;
}

//Modo enfriamiento

void guardianValorCeroContador(){
  if (contadorMaximoSobreCorriente == 0)
  {
    digitalWrite(contadorMaximoAlcanzado, LOW);
    //Apagamos timer de 1Hz
    return;
  }
  
}

void modoEnfriamiento(){
  contadorMaximoSobreCorriente--;
  estadoEnfriamiento = !estadoEnfriamiento;
  digitalWrite(pinIndicadorEnfriamiento, estadoEnfriamiento);
  guardianValorCeroContador();
}

void revisarSobreCorriente(){
  float relacionDeCorriente = leerRelacionDeCorriente();
  if (haySobreCorriente(relacionDeCorriente) && !timer3Activo){
    digitalWrite(pinIndicadorSobreCorriente, HIGH);
    actualizarTimerYContador(relacionDeCorriente);
    //iniciarInterrupcionTimer3();
    return;
  }
  if (!haySobreCorriente(relacionDeCorriente) && timer3Activo){
    digitalWrite(pinIndicadorSobreCorriente, LOW);
    //Si no hay sobreCorriente desactivamos timer3
    //apagarInterrupcionTimer3();
    //Activamos otro timer para enfriamiento a razón de 1hz
    return;
  }
}