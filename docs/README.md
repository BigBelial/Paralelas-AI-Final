# Informe — Proyecto Final Mancala/Kalah

Este informe está dividido en 8 archivos Markdown temáticos. Cada archivo es autocontenido y puede leerse de forma aislada; las referencias entre archivos usan enlaces relativos.

## Índice

1. [01-arquitectura.md](01-arquitectura.md) — componentes, diagrama Mermaid, API REST, CORS.
2. [02-motor.md](02-motor.md) — reglas Kalah(6,4), Minimax con poda Alfa-Beta.
3. [03-paralelizacion.md](03-paralelizacion.md) — estrategia OpenMP e instrumentación local.
4. [04-despliegue-local.md](04-despliegue-local.md) — Docker Compose + Kubernetes local.
5. [05-despliegue-nube.md](05-despliegue-nube.md) — manifiestos YAML del despliegue gestionado.
6. [06-cicd.md](06-cicd.md) — GitHub Actions + SonarQube.
7. [07-analisis-comparativo.md](07-analisis-comparativo.md) — local vs. nube: latencia y throughput.
8. [08-conclusiones.md](08-conclusiones.md) — limitaciones, retos, trabajo futuro.

## Mapeo criterio de la rúbrica → archivo

| Criterio de la rúbrica | Archivo donde se evalúa |
|---|---|
| Motor de Mancala: corrección | [02-motor.md](02-motor.md) |
| Paralelización con OpenMP | [03-paralelizacion.md](03-paralelizacion.md) |
| Instrumentación local | [03-paralelizacion.md](03-paralelizacion.md) |
| Separación de componentes | [01-arquitectura.md](01-arquitectura.md) |
| Despliegue local | [04-despliegue-local.md](04-despliegue-local.md) |
| Despliegue en la nube con Kubernetes | [05-despliegue-nube.md](05-despliegue-nube.md) |
| CI/CD y calidad de código | [06-cicd.md](06-cicd.md) |
| Análisis comparativo local vs. nube | [07-analisis-comparativo.md](07-analisis-comparativo.md) |
| Claridad de explicaciones | transversal a todos los archivos |
| Conclusiones | [08-conclusiones.md](08-conclusiones.md) |

## Notas de formato

- Todos los diagramas se generan con **Mermaid** embebido en el Markdown — no imágenes.
- La notación matemática usa **LaTeX** embebido (`$...$` inline, `$$...$$` display).
- Las capturas de pantalla (perf, htop, kubectl, dashboards) sí se aceptan como **evidencia experimental**, nunca como sustituto de un diagrama.
