#pragma once

#include <Arduino.h>

// Control page served at / (it uses the API below)
const char INDEX_HTML[] PROGMEM = R"html(<!doctype html>
<html><head><meta name="viewport" content="width=device-width,initial-scale=1"><title>Christmas matrix</title>
<style>
body{font-family:sans-serif;background:#0b1a2e;color:#eee;max-width:480px;margin:auto;padding:1em}
h1{color:#ff5252}label{display:block;margin-top:1.2em}input,select,button{font-size:1em;width:100%;box-sizing:border-box;margin-top:.3em}
button{padding:.6em;background:#1b7f3a;color:#fff;border:0;border-radius:6px}small{color:#9ab}
</style></head><body>
<h1>&#127876; Christmas matrix</h1>
<label><input type="checkbox" id="on" style="width:auto"> Display on</label>
<label>Brightness <span id="bv"></span><input type="range" id="brightness" min="5" max="255"></label>
<label>Scene<select id="scene"></select></label>
<label>Message<input id="message" maxlength="200"></label>
<label><input type="checkbox" id="countdown" style="width:auto"> Christmas countdown</label>
<label>Power limit (mA)<input type="number" id="max_milliamps" min="1000" max="60000" step="500"></label>
<button id="save">Save message</button>
<p><small id="status"></small></p>
<script>
const $=id=>document.getElementById(id);
function show(s){
  $('on').checked=s.on;$('brightness').value=s.brightness;$('bv').textContent=s.brightness;
  if(!$('scene').options.length){$('scene').add(new Option('Rotate all scenes',-1));s.scenes.forEach((n,i)=>$('scene').add(new Option(n,i)));}
  $('scene').value=s.scene;$('countdown').checked=s.countdown;$('max_milliamps').value=s.max_milliamps;
  if(document.activeElement!==$('message'))$('message').value=s.message;
  $('status').textContent='Showing: '+s.scenes[s.current_scene]+' | estimated current: '+s.estimated_milliamps+' mA';
}
function set(q){fetch('/api/set?'+q).then(r=>r.json()).then(show);}
$('on').onchange=e=>set('on='+(e.target.checked?1:0));
$('brightness').oninput=e=>$('bv').textContent=e.target.value;
$('brightness').onchange=e=>set('brightness='+e.target.value);
$('scene').onchange=e=>set('scene='+e.target.value);
$('countdown').onchange=e=>set('countdown='+(e.target.checked?1:0));
$('max_milliamps').onchange=e=>set('max_milliamps='+e.target.value);
$('save').onclick=()=>set('message='+encodeURIComponent($('message').value));
function poll(){fetch('/api/state').then(r=>r.json()).then(show);}
poll();setInterval(poll,3000);
</script></body></html>)html";