```
 ________  ________  ________   _______   ________ _________  ________          ___   ___     
|\   ____\|\   __  \|\   ___  \|\  ___ \ |\   ____\\___   ___\\   __  \        |\  \ |\  \    
\ \  \___|\ \  \|\  \ \  \\ \  \ \   __/|\ \  \___\|___ \  \_\ \  \|\  \       \ \  \\_\  \   
 \ \  \    \ \  \\\  \ \  \\ \  \ \  \_|/_\ \  \       \ \  \ \ \   __  \       \ \______  \  
  \ \  \____\ \  \\\  \ \  \\ \  \ \  \_|\ \ \  \____   \ \  \ \ \  \ \  \       \|_____|\  \ 
   \ \_______\ \_______\ \__\\ \__\ \_______\ \_______\  \ \__\ \ \__\ \__\             \ \__\
    \|_______|\|_______|\|__| \|__|\|_______|\|_______|   \|__|  \|__|\|__|              \|__|
```
#
Este proyecto final de Redes de computadores trata de implementar un servidor y un cliente para jugar al juego Conecta 4. 

En este proyecto existen tres partes: objetivo1, objetivo2 y objetivo3.

El objetivo1 trata de un servidor que espera la conexión de dos clientes para jugar una partida de Conecta 4.
El objetivo2 es igual al objetivo1 excepto por dos cosas: la primera es que el servidor solo admite una conexión; la segunda es que el servidor jugara como contricante del cliente con un "bot", una inteligencia artificial.
El objetivo 3 es igual al objetivo1 pero esta vez se muestra el tablero de juego a los dos jugadores y en el servidor. También se da a elegir al segundo jugador si quiere jugar o si quiere que un bot lo haga por el.

En todos los juegos se elige al azar el primer cliente que empezará la partida.

Si quieres saber más sobre el juego Conecta 4 ve al siguiente [enlace](https://es.wikipedia.org/wiki/Conecta_4).


## Instrucciones

Estas instrucciones servirán para obtener una copia del proyecto y poder utilizarlo.


### Instalación 🔧

Descarga el repositorio como cualquier otro repositorio de git.
```
git clone   
```

Utiliza el archivo Makefile facilitado en el repositorio para compilar el código de los programas.

Para compilar todos los objetivos:
```
make all
```
Para compilar el objetivo1:
```
make objetivo1 
```
Para compilar el objetivo2:
```
make objetivo2 
```
Para compilar el objetivo3:
```
make objetivo3 
```
## Requisitos

Los requisitos para la correcta ejecución de los programas son los siguientes:

* Cualquiera de los servidores han de ejecutarse con tres argumentos: el puerto, las filas y las columnas del tablero. Las filas han de ser mayor o igual 6 y las columnas mayor o igual a 7.
* La primera vez que se ejecute cualquiera de los clientes serán necesarios tres argumentos: la IP del servidor, el puerto y el nombre de usuario deseado. En la segunda ejecución y en las demás ya no sera necesario el tercer argumento.
* Para poder jugar es necesario que los clientes se ejecuten en distintos dispositivos o, en el caso de utilizar el mismo, en carpetas separadas. El motivo es que solamente se admite un cliente por dispositivo.
* Se pueden ejecutar tantos servidores como se consideren necesarios. Pero han de ejecutarse en puertos diferentes. De lo contrario el servidor no se ejecutará.
* Para parar la ejecución de cualquiera de los servidores es necesario pararlo manualmente con la señal SIGINT accionando Ctrl + c. Cada servidor, una vez terminada la partida, esperará mas conexiones (una o dos dependiendo del servidor).

## Jugar al juego ⚙️

Para poder jugar ha de ejecutarse antes el servidor correspondiente al tipo de partida que se quiera tener. Quedan explicados cada uno de ellas al principio de este archivo. 

Así pues, aquí quedan todos los comandos para jugar en modo de ejemplo:

* Objetivo1:
```
./servidor1 1024 6 7
./cliente1 192.168.0.27 1024 Adrian
./cliente1 192.168.0.27 1024 Jon
```
* Objetivo2:
```
./servidor2 1024 6 7
./cliente2 192.168.0.27 1024 Adrian
```
* Objetivo3:
```
./servidor3 1024 6 7
./cliente3 192.168.0.27 1024 Adrian
./cliente3 192.168.0.27 1024 Jon
```
Como se explica anteriormente es necesario que los clientes estén en dispositivos diferentes o bien ejecutandose en carpetas diferentes.

## Comentarios del proyecto

La verdad es que ha sido bastante tedioso de programar. Consideramos que el lenguaje utilizado no es el más adecuado para este tipo de proyectos ya que da auténticos quebraderos de cabeza. Sobre todo la comunicación de los sockets a tan bajo nivel. Ha sido divertido e interesante ver como funcionan realmente los programas en red. Aún así creemos que un lenguaje como Java habría sido mejor opción que C.

### Dificultades de cada objetivo

* El objetivo 1 es el que dio mas problemas. No había manera de que se conectaran dos clientes a la vez. Al final decidimos hacer uso de threads y una tabla de clientes para almacenar el descriptor de fichero de cada cliente. 
* El objetivo 2 también nos dio problemas, pero no de redes. El mayor "problema" fue la implementación de la IA. No creemos que sea correcto que se evalúe parte del proyecto con una IA si la asignatura es de redes.
* El objetivo 3 no fué demasiado difícil. Nos costó muy poco crear una función que imprimiese por pantalla el tablero.

#### Resumen
En resumen, recomendaríamos que en los siguientes años el proyecto se centre más en la implementación de código orientado a redes en vez de evaluar conecptos que no tienen que ver con esta asignatura. Como implementar una IA, crear una interfaz de juego, etc.

## Construido con 🛠️

* [C](https://es.wikipedia.org/wiki/C_(lenguaje_de_programaci%C3%B3n)) - Lenguaje de programación C. 

## Autores ✒️

* **Adrian Maiza** - [Maizabros](https://github.com/maizabros)
* **Jon Urtasun** 

