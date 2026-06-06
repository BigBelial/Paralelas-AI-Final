// Script para k6: alternativa a wrk que ya reporta percentiles p50/p95.
//
// Uso:
//   BASE=http://<host>:8000 k6 run move.js
//
// Reporta http_req_duration (p50/p95) y http_reqs (throughput). Usa la misma
// posición y profundidad que post_move.lua para comparar local vs. nube.

import http from 'k6/http';
import { check } from 'k6';

export const options = {
  vus: 50,
  duration: '30s',
};

const BASE = __ENV.BASE || 'http://localhost:8000';

const payload = JSON.stringify({
  board: [4, 4, 4, 4, 4, 4, 0, 4, 4, 4, 4, 4, 4, 0],
  side: 0,
  algo: 'alphabeta',
  depth: 8,
  threads: 2,
});

const params = { headers: { 'Content-Type': 'application/json' } };

export default function () {
  const res = http.post(`${BASE}/move`, payload, params);
  check(res, { 'status 200': (r) => r.status === 200 });
}
