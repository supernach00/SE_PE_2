

# Tareas

La arquitectura del proyecto mantiene una tarea principal (uiTask) que se comunica constantemente con las demás tareas a través de un sistema de colas. 
Esta tarea se encarga de procesar y mostrar en pantalla datos al usuario, y a su vez que controla también la FSM que define el estado actual del sistema en función a la información recibida por dichas colas.

En el siguiente diagrama se muestra de manera gráfica las distintas tareas y la comunicación entre ellas:

[DIAGRAMA]



##### inputsTask:

Es la tarea encargada de tomar lecturas de los inputs del usuario (botón y encoder).

En su **setup**, comienza a correr el timer1, que se encuentra en modo encoder. Inicializa variables para almacenar los datos de lectura y para el debouncer.
En su **bucle**, realiza secuencialmente 2 lecturas:
	- Lee el encoder, mediante lectura del contador del timer1.
	- Lee el botón, leyendo el pin correspondiente.
Para cada lectura, si corresponde, se envía una variable del tipo _Evento_e_ indicando la presión del botón o el movimiento adecuado del encoder, a través de la cola **uiQueue**.

**Período:** fijo de 20ms, elegido en base a la necesidad de una respuesta rápida a los inputs del usuario. 

**Uso del stack:** No es una tarea que haga uso intensivo de la memoria, puesto que no utiliza buffers muy grandes (solo para almacenar lecturas y eventos) y la utilización de una cola (este es el mayor uso). 

120Bytes TCB + 512Bytes (asingación mínima permitida) = TOTAL 623 Bytes
##### monitorTask: 

Es la tarea encargada de monitorear información del sistema y las tareas (stack, factor de utilización, heap), y de enviar esta información a uiTask junto con una alerta si se exceden los límites predefinidos.

En su **setup**, inicializa los buffers necesarios para almacenar dichos datos 
En su **bucle¨**, se realizan las siguiente
	- El factor de utilización del sistema, mediante hooks y calculando el complemento del factor de la tarea idle.
	- El mínimo valor libre del heap del sistema hasta el momento; con la función **xPortGetMinimumEverFreeHeapSize()**
	- El heap libre actual del sistema; con la función **xPortGetFreeHeapSize()**.
	- El mínimo valor libre del stack de cada tarea hasta el momento; con la función **osThreadGetStackSpace()**.

**Comunicación**: Luego de las lecturas, se almacena toda esta información en un dato del tipo _MonitorData_t_ y lo envía por la cola **monitorQueue**.
Además, se realiza un checkeo para verificar que los valores medidos no sobrepasen los límites definidos, en cuyo caso se comunica una alerta con un _Evento_t_ a través de la cola de eventos **uiQueue**.

**Período:** fijo de 50ms basado en las especificaciones.

**Uso del stack:** Tampoco es una tarea que haga uso intensivo de la memoria, puesto que no utiliza buffers muy grandes (solo para almacenar algunos datos de 32 bits y eventos) y la utilización de dos colas (este es el mayor uso). 
Se realizan llamadas a funciones del OS que podrían seguir llamando funciones y requerir más stack, pero se verificó con el stack analyzer que no son fuciones demandantes.

120Bytes TCB + 512Bytes (asignación mínima permitida) = TOTAL 632 Bytes

##### muestreoTask:

Esta tarea se encarga de configurar las resistencias de medición y el timer3 para el modo trigger externo del ADC, iniciar conversiones de DMA, y de comunicar las muestras leídas la tarea **uiTask**.

En su **setup**, declara y inicializa variables locales de configuración (resistencia y base de tiempo del timer), y luego inicia el timer3 y habilita las resistencias según dicha configuración.

En el **bucle**, verifica si está en el estado de medición. Si no lo está, deja un delay de 200ms antes de volver a checkear. En caso de estar en el estado de medición, actua de maneras diferentes según la variable global de configuración.

Configuración en modo R:

Se realizan 256 conversiones y se llena un buffer de dicho tamaño.
	- Se define la resistencia en 10kohm
	- Configura el timer3 para que genere muestras con el período más corto (100us), reinicia el contador e inicializa el timer.
	- Inicializa el ADC con el modo DMA. El objetivo es que se llene el buffer de 256 muestras sin utilizar recursos del procesador.
	- Llama a la función del OS **ulTaskNotifyTake()**, que queda esperando una notifiación de parte del DMA (enviada desde la interrupción que se genera cuando se llena el buffer).

Cuando la tarea recibe la notificación:
	- Calcula la acumulación de las 256 muestras y las diezma, quedando un valor resultante de 16 bits
	- Se verifica si este valor excede el
##### uiTask:
En general, es la tarea que se encarga de recibir eventos (como el movimiento del encoder, el botón, o alertas de sobrepaso de stack) por la cola **uiQueue** y en consecuencia actualizar una variable struct que contiene la información del sistema (principalmente menúes de pantalla).
Por otra parte, también se encarga de recibir datos de muestreo a ttravés de la cola **muestrasQueue**.

Por último, en base a la información del sistema y de los datos de muestreo, es la única tarea que imprime en la pantalla oled la información correspondiente.

En el **setup** se declaran e inicializan buffers para contener los mensajes de las colas, variables de iteración, buffers de evento.

En el **bucle**:
	- Se leen todos los elementos de la cola de eventos **uiQueue**, y se llama a la función **ui_FSM_siwtch()** para actualizar la máquina de estados.
	- Se lee la cola de datos de monitor **monitorQueue**, y se actualiza el buffer local con esa información.
	- Se lee la cola de muestras **muestreoQueue**, se actualiza el buffer local con la última muestra. 
	- Por último, con toda la información disponible, se imprime a la pantalla con la función **ui_update_oled()**, que internamente decide finalmente qué mostrar según el estado del sistema.

**Período:** 100ms, actualizar el sistema y la pantalla más rápido sería innecesario.

**Uso del stack:** Los bitmaps utilizados están almacenados en flash como constantes, por lo que no requieren stack para su utilización. De igual manera, se espera que utilice más stack que el resto de las tareas, debido a la utilización de la libreria SSD1306, funciones de string.h, sumado a los buffers locales y variables utilizadas.

120Bytes TCB + 1200Bytes (asginado basado en stack analyzer) = TOTAL 1320Bytes