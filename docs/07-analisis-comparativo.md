# 07 — Análisis Comparativo: Local vs. Nube

## Configuración del experimento

- **Local**: una sola instancia del backend en máquina física o VM, variando `OMP_NUM_THREADS ∈ {1, 2, 4, 8}`. Únicamente motor Alfa-Beta a profundidad fija.
- **Nube**: el mismo backend en Kubernetes con dos configuraciones de réplicas, $r \in \{1, 3\}$. `OMP_NUM_THREADS` fijo a un valor moderado (e.g. `2`) y se varía $r$.
- **Carga sintética**: peticiones concurrentes con `wrk`, `k6` o `ab`. Cada petición envía un estado de tablero y una profundidad fija.

Imagen del motor y profundidad de búsqueda se mantienen idénticas en ambos entornos.

## Métricas reportadas

- **Latencia por petición**: percentiles $p_{50}$ y $p_{95}$.
- **Throughput**: peticiones/segundo sostenidas.

## Resultados — Local (variando hilos)

Una sola instancia del backend (`docker compose up`), misma posición y
profundidad (`depth=10`), variando el número de hilos OpenMP del motor.

**Nota metodológica (desviación justificada, regla 7):** el conteo de hilos del
motor lo fija el campo `threads` del payload de `POST /move`, no la variable
`OMP_NUM_THREADS`. En [`alphabeta.cpp`](../motor/src/alphabeta.cpp) el
`#pragma omp parallel for num_threads(cfg.threads)` **sobrescribe**
`OMP_NUM_THREADS`, y `cfg.threads` proviene de la petición
([`api.cpp`](../motor/src/api.cpp)). Por eso el barrido de hilos se hace
variando `threads` en la carga, manteniendo idénticas la posición y la
profundidad. La columna refleja ese conteo de hilos por petición.

- Carga: `k6`, 50 usuarios virtuales (VUs) concurrentes, 30 s por corrida.
- Entorno: máquina con 8 núcleos, los 3 contenedores en Docker Compose.
- Posición: tablero inicial `[4,4,4,4,4,4,0,4,4,4,4,4,4,0]`, `depth=10`.

| Hilos motor (`threads`) | p50 [ms] | p95 [ms] | Throughput [req/s] |
|---|---|---|---|
| 1 | 104.9 | 122.1 | 465.8 |
| 2 | 150.3 | 167.2 | 329.4 |
| 4 | 149.7 | 166.6 | 331.6 |
| 8 | 153.9 | 172.3 | 322.4 |

Evidencia de una corrida de carga con `k6` (50 VUs, `threads=4`, `depth=10`).
La captura corresponde a una corrida corta de verificación de 15 s
(p50 ≈ 149 ms, p95 ≈ 183 ms, ≈ 327 req/s), del mismo orden que la fila
`threads=4` de la tabla, medida con corridas de 30 s:

![Salida de k6 en la prueba de carga local](img/carga.png)

## Resultados — Nube (variando réplicas, `OMP_NUM_THREADS=2`)

Mismo backend en Kubernetes, hilos fijos en 2, variando réplicas. **Llenar con la
salida real medida en el clúster del grupo.**

| Réplicas $r$ | p50 [ms] | p95 [ms] | Throughput [req/s] |
|---|---|---|---|
| 1 | _(carga)_ | _(carga)_ | _(carga)_ |
| 3 | _(carga)_ | _(carga)_ | _(carga)_ |

## Observación cualitativa

Los números locales son contraintuitivos a primera vista pero coherentes con el
hardware: con 50 peticiones concurrentes sobre 8 núcleos, el mejor resultado se
obtiene con **1 hilo por petición** (p50 = 104.9 ms, throughput = 465.8 req/s).
Al subir a 2, 4 u 8 hilos por petición el throughput **cae** a ~330 req/s y la
latencia p95 sube de 122 ms a ~170 ms, estabilizándose porque la máquina ya está
saturada: la concurrencia de peticiones, no la búsqueda individual, ocupa todos
los núcleos. Cada hilo extra por petición compite por CPU ya ocupada y añade el
sobrecosto del root parallelism (más nodos explorados al perder podas, ver
[03-paralelizacion.md](03-paralelizacion.md)).

La conclusión: el **escalado vertical** (más hilos por pod) solo conviene cuando
una petición se atiende prácticamente en aislamiento y hay núcleos libres —ahí el
speedup de la [sección 03](03-paralelizacion.md) sí reduce la latencia individual—.
Bajo carga concurrente alta con CPU saturada, conviene el **escalado horizontal**
(más réplicas/núcleos sirviendo peticiones de 1 hilo en paralelo), que es lo que
ofrece Kubernetes con réplicas del backend. Las tablas confirman que, a igual
hardware, repartir el trabajo en más peticiones de 1 hilo rinde más que pocas
peticiones de muchos hilos.

## Comando de carga utilizado

Los scripts están en [`deploy/local/loadtest/`](../deploy/local/loadtest/). La
tabla local se generó con `k6` ejecutado vía su imagen Docker (sin instalar nada
en el host), barriendo `THREADS ∈ {1,2,4,8}` a `DEPTH=10`:

```bash
cd deploy/local/loadtest
for T in 1 2 4 8; do
  docker run --rm -i -v "$PWD/move.js:/move.js:ro" \
    -e BASE=http://host.docker.internal:8000 -e THREADS=$T -e DEPTH=10 -e DURATION=30s \
    grafana/k6 run /move.js
done
```

Con `k6` instalado nativamente el comando equivalente es:

```bash
BASE=http://localhost:8000 THREADS=4 DEPTH=10 k6 run deploy/local/loadtest/move.js
```

Alternativa con `wrk` (script `post_move.lua`, ajustar `threads` dentro del Lua):

```bash
wrk -t4 -c50 -d30s -s deploy/local/loadtest/post_move.lua http://<host>:8000/move
```

> Para la nube, repetir la misma carga (`DEPTH=10`, `THREADS=2` fijo) contra la
> IP/Ingress del backend en Kubernetes variando las réplicas `r ∈ {1,3}` y pegar
> los p50/p95/throughput en la tabla de arriba.
