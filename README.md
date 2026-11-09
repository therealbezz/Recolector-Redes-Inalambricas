# Recolector-Redes-Inalambricas


Readme del programa

## Entrega 2

Recolector de redes inalambricas en C. Ejecutar desde la raiz del proyecto:

```sh
make
./main
```

Requiere GCC y Make en Linux (laboratorio UC3M, maquina virtual o WSL).
El Makefile usa `-std=c11 -Wall -Wextra -pedantic`. No requiere bibliotecas
externas. Python 3 solo es necesario para el empaquetado:

```sh
make package
make clean
```

El ZIP se genera en `dist/entrega2_wifi.zip`. Contiene una carpeta
`entrega2_wifi/` con fuentes, cabeceras, Makefile, este README, los 21
ficheros originales de entrada. No incluye `.git`, objetos
ni ejecutables. Descomprimir, entrar en esa carpeta y ejecutar `make`.

### Operaciones

| Opcion | Operacion | Comportamiento |
| --- | --- | --- |
| 1 | wificollector_quit | Confirmar con `s`; `N` vuelve al menu |
| 2 | wificollector_collect | Leer todos los puntos de una celda 1-21 |
| 6 | wificollector_sort | Mostrar los puntos por calidad decreciente |
| 9 | wificollector_display | Mostrar todos los puntos de una celda guardada |
| 10 | wificollector_display_all | Mostrar todo en orden de recoleccion |

Las opciones 3, 4, 5, 7 y 8 no forman parte de esta entrega. Las confirmaciones
aceptan exactamente `s` o `N`. Se repiten las preguntas tras entradas
invalidas; el fin de entrada termina el programa limpiamente.

### Datos y decisiones

- El array dinamico empieza con cinco posiciones y crece de cinco en cinco
  mediante `realloc`. Se libera al salir, incluso al terminar la entrada.
- Cada cadena tiene 80 posiciones: 79 caracteres mas el terminador nulo.
- Los ficheros se buscan en `cells/info_cell_N.txt`, con N entre 1 y 21.
- Cada bloque `Cell` es un punto de acceso independiente. Los 21 ficheros
  originales contienen 47 puntos de acceso en total.
- Se guardan celda, MAC, ESSID, modo, canal, cifrado y calidad (dos enteros).
  Frequency y Signal level se ignoran; no se limita el canal a 1-14.
- Volver a recolectar una celda agrega sus observaciones de nuevo, sin
  deduplicar. No se guarda informacion al terminar el programa.
- Los ficheros se validan completos en un array temporal dinamico.
  Un error de formato, fichero vacio, fallo de E/S o falta de memoria
  deja todos los datos anteriores intactos; no se agregan lotes parciales.
- Una celda no recolectada produce un mensaje de error y ofrece imprimir
  otra; una coleccion vacia muestra un mensaje informativo.
- La opcion 6 compara la fraccion de calidad actual sobre maxima. Conserva
  el orden de recoleccion en los empates y en las opciones 9 y 10.

### Modulos e interfaces

- `src/main.c`: punto de entrada y bucle principal.
- `src/menu.c` y `include/menu.h`: menu, despacho y preguntas de cada operacion.
- `src/input.c` y `include/input.h`: lectura de lineas, enteros y confirmaciones.
- `src/collector.c` y `include/collector.h`: datos, validacion y recoleccion.
  `collector_collect(cell, path)` devuelve el estado y el numero agregado.
- `src/display.c` y `include/display.h`: salida de una celda o de toda la coleccion.

El recolector gestiona la memoria de los arrays y devuelve por valor el
resultado de cada carga. La lectura usa `FILE *` de la biblioteca estandar.

### Reparto propuesto y flujo GitHub

| Responsable | Trabajo previsto |
| --- | --- |
| Dani | Menu, entrada y confirmaciones |
| Persona 2 | Tipos compartidos, parser y capacidad |
| Persona 3 | Display, Makefile, README y comprobaciones de integracion |

Para el trabajo individual, crear `feature/menu`, `feature/collect` y
`feature/display` desde la base compartida acordada. Para nuevas tareas
despues de integrar la implementacion, actualizar primero `main`:

```sh
git switch main
git pull --ff-only origin main
git switch -c feature/menu
```

Cada persona hace commits pequeños, sube su rama y abre una pull request
hacia `main`. Otra persona revisa los cambios antes de fusionar.
No trabajar todos sobre los mismos ficheros sin coordinar los cambios.
No subir ejecutables ni objetos. Si `main` no existe localmente, obtenerlo
del remoto antes de seguir estos comandos.

### Validacion y entrega

Antes de entregar, probar una extraccion nueva del ZIP en el laboratorio
UC3M o en la maquina virtual proporcionada: `make` y `./main`.
La validacion local en WSL no sustituye esta comprobacion del equipo.
Entrega 2: 13 de noviembre de 2026, 23:55 (Europe/Madrid), en Aula Global.
