# <span style="color: #24b1e7;">Proyecto de Sistemas Operativos: Máquina de Vending</span>


¡Bienvenido al repositorio de nuestra máquina de vending! En este proyecto, hemos diseñado un sistema concurrente que simula una máquina de vending utilizando un modelo de productores, consumidores y un facturador. Este sistema procesa datos de consumo a partir de varios archivos de entrada de productos.

## <span style="color: #00ff7f;">Descripción General</span>

El objetivo de este proyecto es diseñar un sistema concurrente que pueda gestionar el consumo de una máquina de vending con una única línea de productos. El sistema procesa caracteres de varios archivos de entrada de productos, donde cada carácter representa un tipo de producto específico.

### Características principales:

- **Modelo de Productores-Consumidores**: Los productores generan productos y los colocan en un búfer circular, mientras que los consumidores consumen los productos de ese búfer.
  
- **Facturador**: El facturador recopila información de los consumidores y calcula estadísticas sobre el consumo total, productos por proveedor y el consumidor más activo.

- **Sincronización de Hilos**: Utiliza hilos en UNIX para gestionar la concurrencia y semáforos para controlar el acceso a la memoria compartida.

## <span style="color: #00ff7f;">Instrucciones de Uso</span>

Para ejecutar el programa, sigue estos pasos:

1. **Compila el código**: Asegúrate de tener un compilador de C instalado. Compila el programa utilizando un comando como `gcc -o vending_machine main.c` en la terminal.

2. **Ejecuta el programa**: Una vez compilado, ejecuta el programa con los siguientes parámetros:

   ```shell
   ./vending_machine <ruta> <archivoSalida> <T> <P> <C>
   ```
- ruta: Ruta a los archivos de entrada de productos.
- archivoSalida: Nombre del archivo de salida para los resultados finales.
- T: Tamaño del búfer circular (entre 1 y 5000).
- P: Número de proveedores (entre 1 y 7).
- C: Número de clientes consumidores (entre 1 y 1000).

## <span style="color: #00ff7f;">Ejemplos de Ejecución</span>
A continuación, se presentan algunos ejemplos de ejecución del programa:
   ```shell
   ./vending_machine /ruta/a/ficheros salida.txt 100 2 3
   ```
Este comando ejecuta el programa con un búfer circular de tamaño 100, 2 proveedores y 3 consumidores. Los resultados se guardarán en salida.txt.

## <span style="color: #00ff7f;">Contribuciones</span>
¡Nos encantaría recibir contribuciones para mejorar este proyecto! Si deseas contribuir, abre un pull request o una issue para discutir las mejoras que te gustaría hacer.

## <span style="color: #00ff7f;">Licencia</span>
Este proyecto está bajo la Licencia MIT.

## <span style="color: #00ff7f;">Autor</span>
- Jimena Rioja
- Contacto: jimenarioja30@gmail.com