# Proyecto Final: Motores Paralelos de Mancala (Kalah) y Despliegue en Kubernetes

Curso: Infraestructuras Paralelas y Distribuidas — Universidad del Valle
Docente: Carlos Andrés Delgado S., MSc — carlos.andres.delgado@correounivalle.edu.co
Fecha de entrega: miércoles, 10 de junio de 2026, 17:58:59 (push a la rama principal)

## Integrantes del grupo

| Nombre completo | Código | Correo institucional |
|---|---|---|
| Juan Jose Ospina Sanchez | 2559711 | juan.jose.ospina@correounivalle.edu.co |
| Juan David Quintero Garcia | 2559710 | juan.quintero.garcia@correounivalle.edu.co |
| ... | ... | ... |
| ... | ... | ... |

> Completar la tabla con los datos reales antes de la entrega. No completar el README.md penaliza un 10% sobre la nota final del proyecto.

## Descripción

Motor de juego para **Kalah(6,4)** implementado en C++ con **Minimax y poda
Alfa-Beta** (búsqueda adversaria exhaustiva), paralelizado con OpenMP.

Expuesto como servicio HTTP vía wrapper Python/FastAPI, empaquetado en contenedores y orquestado con Kubernetes (local + nube).

## Arquitectura (3 contenedores)

```
[ Navegador ] ──► [ frontend (nginx) ] ──► [ backend (FastAPI) ] ──► [ motor (C++/OpenMP) ]
                       puerto 8080              puerto 8000           red interna
```

Cada componente vive en su propio contenedor y se comunican por la red del clúster. El motor **no** se enlaza al backend con pybind11/ctypes — la separación es a nivel de contenedor.

## Build y ejecución local

### Opción 1 — Docker Compose (recomendado)

```powershell
cd deploy\local
docker compose up --build
```

Esto levanta los 3 contenedores y deja disponible:
- Frontend: http://localhost:8080
- Backend: http://localhost:8000 (`/healthz`, `/readyz`, `/metrics`, `POST /move`)

### Opción 2 — Componentes por separado

Motor (C++):
```powershell
cd motor
cmake -S src -B build
cmake --build build
.\build\mancala_motor.exe
```

Backend (Python):
```powershell
cd backend
pip install -r requirements.txt
uvicorn app.main:app --reload --host 0.0.0.0 --port 8000
pytest tests/
```

Frontend: abrir `frontend/public/index.html` directamente, o servir con `python -m http.server 8080` desde `frontend/public/`.

## Estado actual

Implementado:

- **Motor C++/OpenMP**: reglas Kalah(6,4), Minimax+Alfa-Beta con root parallelism, servidor HTTP propio, modo benchmark CLI, suite de tests unitarios.
- **Backend FastAPI**: API REST con schemas pydantic, CORS explícito, delega al motor por HTTP, 10/10 tests pasan.
- **Frontend HTML/JS + nginx**: tablero Kalah interactivo que consume `/move`.
- **Docker Compose**: levanta los 3 contenedores con un comando.
- **Manifiestos Kubernetes**: en `deploy/local/k8s/` (kind/minikube) y `deploy/cloud/` (EKS/AKS/GKE) con requests/limits, probes, ConfigMap, Ingress + HPA.
- **CI/CD**: workflow de GitHub Actions con compilación del motor, tests del motor y del backend, construcción y publicación de imágenes a GHCR (en push a `main`) y escaneo de SonarCloud declarado en YAML.

- **Instrumentación (docs/03)**: barrido T(p)/S(p)/E(p) a `depth=8` y `depth=12`,
  tabla de pérdida de podas (49 % más nodos en paralelo), gráfica de speedup y
  capturas de `/usr/bin/time -v` y `htop`. **Hecho.**

Pendiente (requiere cuentas del grupo / despliegue, no se puede inventar):

- Elegir proveedor cloud (este proyecto usa **GCP GKE**), publicar las imágenes con
  tag inmutable en el registro y reemplazarlo en `deploy/cloud/*.yaml`, desplegar y
  pegar la salida de `kubectl get pods,svc,deploy` en `docs/05-despliegue-nube.md`.
- Crear el proyecto en SonarCloud, cargar `SONAR_TOKEN` como *Repository secret* y
  ajustar `sonar.projectKey` / `sonar.organization` en `.github/workflows/ci.yml`.
- Correr la prueba de carga (`deploy/local/loadtest/`) y pegar p50/p95/throughput
  en `docs/07-analisis-comparativo.md` (local variando hilos, nube variando réplicas).
- Completar la tabla de integrantes de arriba (no hacerlo penaliza un 10%).

## Configuración del frontend según el entorno

El cliente del navegador llama al backend **directamente** (con CORS). La URL del
backend se resuelve en este orden:

1. `window.MANCALA_API_BASE` si está definida en `frontend/public/config.js`.
2. Si no, `http://<host-actual>:8000` (válido para Docker Compose).

Ajusta `config.js` según dónde corra el backend:

- Docker Compose: dejarlo vacío (usa el puerto 8000 del host).
- Kubernetes local (NodePort): `http://<IP-del-nodo>:30080`.
- Nube (Ingress en el mismo dominio): `/api`.

## Informe

El informe completo vive en [`docs/`](docs/README.md), dividido en 8 archivos Markdown temáticos según la especificación del curso.

## Estructura del repositorio

```
.
├── README.md                  # este archivo
├── docs/                      # informe (9 archivos Markdown)
├── motor/                     # contenedor 1: motor C++/OpenMP
├── backend/                   # contenedor 2: wrapper FastAPI
├── frontend/                  # contenedor 3: cliente web (nginx)
├── deploy/
│   ├── local/                 # docker-compose + manifiestos kind/minikube
│   └── cloud/                 # manifiestos YAML del despliegue en la nube
└── .github/workflows/         # pipelines CI/CD + SonarQube por YAML
```
