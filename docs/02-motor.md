# 02 — Motor de Juego

## Reglas de Kalah(6,4)

Variante estándar de Mancala con:

- Tablero con **2 filas de 6 hoyos**, **4 semillas** inicialmente en cada hoyo.
- Cada jugador tiene un **kalaha** (almacén) a su derecha.
- En su turno, el jugador toma todas las semillas de uno de sus hoyos y las distribuye una a una en **sentido antihorario**, incluyendo su propio kalaha pero **saltando** el del oponente.
- Si la última semilla cae en el kalaha propio → **turno extra**.
- Si la última semilla cae en un hoyo propio vacío y el hoyo opuesto del rival contiene semillas → el jugador **captura** ambas hacia su kalaha.
- El juego termina cuando uno de los lados queda vacío; el jugador con más semillas en su kalaha gana.

Representación interna: arreglo de 14 enteros `board[0..13]` con orden canónico documentado en [01-arquitectura.md](01-arquitectura.md).

## Algoritmo 1 — Minimax con poda Alfa-Beta

Búsqueda en profundidad fija (`depth`) sobre el árbol de juego.

### Función de evaluación heurística

$$
h(\text{estado}) = (\text{kalaha propio} - \text{kalaha rival}) + \alpha \cdot (\text{semillas lado propio} - \text{semillas lado rival})
$$

con $\alpha \in [0, 1]$ definido por el grupo.

### Pseudocódigo

```text
funcion alphabeta(estado, depth, alpha, beta, maximizando):
    si depth == 0 o estado.terminal():
        retornar h(estado)
    si maximizando:
        valor = -INF
        para cada movimiento legal m en estado:
            valor = max(valor, alphabeta(aplicar(estado, m), depth-1, alpha, beta, false))
            alpha = max(alpha, valor)
            si beta <= alpha: break   # poda beta
        retornar valor
    si no:
        valor = +INF
        para cada movimiento legal m en estado:
            valor = min(valor, alphabeta(aplicar(estado, m), depth-1, alpha, beta, true))
            beta = min(beta, valor)
            si beta <= alpha: break   # poda alpha
        retornar valor
```

### Criterio de corrección

Árboles podados deben producir el **mismo movimiento óptimo** que Minimax sin poda a igual profundidad. Esta equivalencia se valida con la suite de pruebas unitarias.

## Algoritmo 2 — Monte Carlo Tree Search (MCTS) con UCT

Búsqueda estocástica *anytime*: no necesita función de evaluación heurística. Cada movimiento se valora con simulaciones aleatorias (rollouts) hasta el final del juego.

### Política de selección UCT

$$
UCT(n) = \frac{w_n}{N_n} + c \cdot \sqrt{\frac{\ln N_{\text{padre}}}{N_n}}
$$

con $w_n$ las victorias en simulaciones que pasaron por $n$, $N_n$ el número de visitas a $n$, y $c$ una constante de exploración (típicamente $c = \sqrt{2}$).

### Las cuatro fases canónicas

1. **Selección** — descender por UCT hasta una hoja.
2. **Expansión** — agregar un hijo nuevo.
3. **Simulación** — jugar al azar hasta el final.
4. **Retropropagación** — actualizar $w$ y $N$ en el camino recorrido.

### Pseudocódigo

```text
funcion mcts(raiz, simulations):
    para i en 1..simulations:
        nodo = seleccion_uct(raiz)
        hijo = expandir(nodo)
        resultado = rollout(hijo.estado)
        retropropagar(hijo, resultado)
    retornar hijo_de(raiz) con mayor N
```

### Criterio de corrección

MCTS no garantiza el movimiento óptimo: su corrección es **estadística**. Se reporta la **tasa de coincidencia** entre la jugada de MCTS y la jugada óptima de Alfa-Beta sobre el mismo conjunto de posiciones, para presupuestos crecientes de simulaciones.

## Suite de pruebas unitarias

Ubicada en `motor/tests/`. Cómo ejecutarla:

```bash
cd motor
cmake -S src -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

> **Pendiente de completar**: implementación real de las reglas, suite de pruebas concretas, tabla de resultados de coincidencia MCTS vs. Alfa-Beta.
