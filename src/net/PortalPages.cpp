#include "net/PortalPages.h"

namespace PortalPages {

  String gamesPage() {
    return String(R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32 Games</title>
  <style>
    body { font-family: sans-serif; text-align: center; background: #111; color: #eee; margin: 0; padding: 2em; }
    h1 { font-size: 2em; margin-bottom: 0.2em; }
    p.sub { color: #888; margin-top: 0; margin-bottom: 1.5em; font-size: 0.95em; }
    a.game {
      display: block; margin: 1em auto; padding: 1.5em;
      font-size: 1.4em; border-radius: 12px; max-width: 320px;
      text-decoration: none; color: white;
      background: linear-gradient(135deg, #1565c0 0%, #c62828 100%);
      box-shadow: 0 3px 8px rgba(0,0,0,0.4);
    }
    a.game small { display: block; font-size: 0.7em; opacity: 0.85; margin-top: 0.3em; }
    a.game.disabled {
      background: #2a2a2a; color: #666; pointer-events: none;
      box-shadow: none;
    }
  </style>
</head>
<body>
  <h1>Choose a game</h1>
  <p class="sub">Hosted on ESP32-Game</p>
  <a class="game" href="/stickman">
    Stickman Kombat
    <small>2-player fighter</small>
  </a>
  <a class="game disabled">
    More coming soon
    <small>stay tuned</small>
  </a>
</body>
</html>
)HTML");
  }

  String selectionPage() {
    return String(R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Stickman Kombat</title>
  <style>
    body { font-family: sans-serif; text-align: center; background: #111; color: #eee; margin: 0; padding: 2em; }
    h1 { font-size: 1.8em; margin-bottom: 0.1em; }
    p.sub { color: #888; margin-top: 0; margin-bottom: 1.5em; font-size: 0.95em; }
    a.button {
      display: block; margin: 1em auto; padding: 1.5em;
      font-size: 1.5em; border-radius: 12px;
      text-decoration: none; color: white; max-width: 300px;
    }
    .p1 { background: #1565c0; }
    .p2 { background: #c62828; }
    a.back { color: #888; font-size: 0.95em; text-decoration: none; }
  </style>
</head>
<body>
  <h1>Stickman Kombat</h1>
  <p class="sub">Choose your side</p>
  <a class="button p1" href="/p1">Player 1</a>
  <a class="button p2" href="/p2">Player 2</a>
  <p><a class="back" href="/">&larr; back to games</a></p>
</body>
</html>
)HTML");
  }

  String controllerPage(int player) {
    String html = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
  <title>Pad P__P__</title>
  <style>
    html, body { height: 100%; margin: 0; overflow: hidden; }
    body {
      font-family: sans-serif; background: #111; color: #eee;
      display: flex; flex-direction: column;
      padding: 0.5em; box-sizing: border-box;
      user-select: none; -webkit-user-select: none;
      touch-action: none;
    }
    h2 { text-align: center; margin: 0.2em 0; font-size: 1.2em; }

    .pad {
      flex: 1;
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 0.6em;
      align-items: center;
    }

    .slider-wrap {
      display: flex; align-items: center; justify-content: center; height: 100%;
    }
    .slider {
      position: relative;
      width: 94%;
      height: 96px;
      background: #1c1c1c;
      border: 2px solid #333;
      border-radius: 50px;
      touch-action: none;
      overflow: hidden;
    }
    .slider::before {
      content: ""; position: absolute;
      top: 50%; left: 8%; right: 8%; height: 3px;
      background: #2a2a2a; transform: translateY(-50%);
    }
    .slider::after {
      content: ""; position: absolute;
      top: 50%; left: 50%; width: 4px; height: 14px;
      background: #444; transform: translate(-50%, -50%);
      border-radius: 2px;
    }
    .runMark {
      position: absolute; top: 50%; width: 3px; height: 18px;
      background: #ef6c00; transform: translateY(-50%); opacity: 0.6;
      border-radius: 2px;
    }
    .runMark.left  { left: 18%; }
    .runMark.right { right: 18%; }

    .thumb {
      position: absolute;
      top: 50%; left: 50%;
      width: 74px; height: 74px;
      background: #1565c0;
      border-radius: 50%;
      box-shadow: 0 2px 6px rgba(0,0,0,0.5);
      --off: 0px;
      transform: translate(calc(-50% + var(--off)), -50%);
      pointer-events: none;
      transition: background-color 0.1s linear;
    }
    .thumb.running { background: #ef6c00; }

    .buttons {
      height: 100%;
      display: grid;
      grid-template-columns: 1fr 1fr;
      grid-template-rows: 1fr 1fr;
      gap: 0.5em;
      place-items: center;
    }
    .round {
      width: 88%; aspect-ratio: 1;
      max-width: 96px; max-height: 96px;
      min-width: 64px; min-height: 64px;
      border-radius: 50%;
      font-size: 0.95em; font-weight: bold; letter-spacing: 0.04em;
      border: none; color: white;
      touch-action: manipulation;
    }
    .round:active, .round.pressed { filter: brightness(1.35); transform: scale(0.94); }
    .punch { background: #2e7d32; }
    .kick  { background: #ef6c00; }
    .block { background: #6a1b9a; }
    .duck  { background: #00838f; }
  </style>
</head>
<body>
  <h2>Player __P__</h2>
  <div class="pad">
    <div class="slider-wrap">
      <div class="slider" id="slider">
        <div class="runMark left"></div>
        <div class="runMark right"></div>
        <div class="thumb" id="thumb"></div>
      </div>
    </div>
    <div class="buttons">
      <button class="round punch" data-a="punch">PUNCH</button>
      <button class="round kick"  data-a="kick">KICK</button>
      <button class="round block" data-a="block">BLOCK</button>
      <button class="round duck"  data-a="duck_on" data-a-up="duck_off">DUCK</button>
    </div>
  </div>
<script>
(function () {
  const P = __P__;

  function leaveSession() {
    location.href = '/';
  }

  // One WebSocket per controller. The server uses ?p=N to route this
  // socket's messages to the right player. On 'close' (sent by the host
  // when the match is aborted) or any socket close the page bounces back
  // to the games hub.
  let ws;
  let leaving = false;
  function connect() {
    ws = new WebSocket('ws://' + location.hostname + ':81/?p=' + P);
    ws.addEventListener('message', (e) => {
      if (e.data === 'close') { leaving = true; leaveSession(); }
    });
    ws.addEventListener('close', () => { if (!leaving) leaveSession(); });
  }
  connect();

  function send(a) {
    if (ws && ws.readyState === WebSocket.OPEN) ws.send(a);
  }

  const slider = document.getElementById('slider');
  const thumb  = document.getElementById('thumb');
  const DEAD_ZONE = 0.18;
  const RUN_ZONE  = 0.65;

  let dragging = false;
  let activeId = null;
  let offset   = 0;

  function setThumb(o, smooth) {
    offset = Math.max(-1, Math.min(1, o));
    const r = slider.getBoundingClientRect();
    const maxOff = (r.width - thumb.offsetWidth) / 2 - 6;
    thumb.style.transition = smooth ? 'transform 0.18s ease' : 'none';
    thumb.style.setProperty('--off', (offset * maxOff) + 'px');
    const inRunZone = Math.abs(offset) > RUN_ZONE;
    thumb.classList.toggle('running', dragging && inRunZone);
  }

  function offsetFromEvent(e) {
    const r = slider.getBoundingClientRect();
    const thumbR = thumb.offsetWidth / 2;
    const half = (r.width / 2) - thumbR - 6;
    const x = e.clientX - (r.left + r.width / 2);
    return Math.max(-1, Math.min(1, x / half));
  }

  slider.addEventListener('pointerdown', (e) => {
    e.preventDefault();
    dragging = true;
    activeId = e.pointerId;
    try { slider.setPointerCapture(e.pointerId); } catch (_) {}
    setThumb(offsetFromEvent(e), false);
  });
  slider.addEventListener('pointermove', (e) => {
    if (!dragging || e.pointerId !== activeId) return;
    setThumb(offsetFromEvent(e), false);
  });
  function endDrag(e) {
    if (!dragging || e.pointerId !== activeId) return;
    dragging = false;
    activeId = null;
    try { slider.releasePointerCapture(e.pointerId); } catch (_) {}
    setThumb(0, true);
  }
  slider.addEventListener('pointerup', endDrag);
  slider.addEventListener('pointercancel', endDrag);

  // Fire movement at a steady tempo while the thumb is held off-centre.
  // The fighter ignores movement mid-attack, so multi-touch is safe.
  const TEMPO_MS = 110;
  setInterval(() => {
    if (!dragging) return;
    const mag = Math.abs(offset);
    if (mag < DEAD_ZONE) return;
    const right = offset > 0;
    if (mag > RUN_ZONE) send(right ? 'run_right' : 'run_left');
    else                send(right ? 'right'     : 'left');
  }, TEMPO_MS);

  window.addEventListener('resize', () => { if (!dragging) setThumb(0, false); });

  // DUCK is a hold button (data-a-up on release); the others fire on press.
  // Per-button pointer capture is what makes the pad multi-touch.
  document.querySelectorAll('.round').forEach(b => {
    const downAction = b.dataset.a;
    const upAction   = b.dataset.aUp;

    b.addEventListener('pointerdown', (e) => {
      e.preventDefault();
      try { b.setPointerCapture(e.pointerId); } catch (_) {}
      b.classList.add('pressed');
      send(downAction);
    });

    const release = (e) => {
      if (!b.classList.contains('pressed')) return;
      b.classList.remove('pressed');
      try { b.releasePointerCapture(e.pointerId); } catch (_) {}
      if (upAction) send(upAction);
    };
    b.addEventListener('pointerup', release);
    b.addEventListener('pointercancel', release);
  });

  document.addEventListener('gesturestart', (e) => e.preventDefault());
})();
</script>
</body>
</html>
)HTML";
    html.replace("__P__", String(player));
    return html;
  }

}
