window.addEventListener('DOMContentLoaded',()=>{
    // UI
    const bpmInput=document.getElementById('bpmInput');
    const bpmRange=document.getElementById('bpmRange');
    const beatsPerBarSel=document.getElementById('beatsPerBar');
    const beatUnitSel=document.getElementById('beatUnit');
    const startBtn=document.getElementById('startBtn');
    const stopBtn=document.getElementById('stopBtn');
    const bar=document.getElementById('bar');
    const progressFill=document.getElementById('progressFill');
    const drumsToggle=document.getElementById('drumsToggle');
    const drumsVol=document.getElementById('drumsVol');
    const countToggle=document.getElementById('countToggle');
    const countdownDiv = document.getElementById('countdown');

    // Auto‑BPM
    const autoGroup=document.getElementById('autoGroup');
    const autoToggle=document.getElementById('autoBpmToggle');
    const autoMin=document.getElementById('autoMin');
    const autoMax=document.getElementById('autoMax');
    const autoStep=document.getElementById('autoStep');
    const autoEvery=document.getElementById('autoEvery');
    const autoUnit=document.getElementById('autoUnit');
    const autoLoop=document.getElementById('autoLoop');
    const autoReverse=document.getElementById('autoReverse');
    const autoResetBtn=document.getElementById('autoResetBtn');
    const nextStepTimer=document.getElementById('nextStepTimer');

    // === Profiles ===
    const profiles = Array.from({length: 5}, () => ({
        bpm: 120,
        beatsPerBar: 4,
        beatUnit: 4,
        drumsToggle: true,
        drumsVol: 0.8,
        countToggle: true,
        autoBpmToggle: false,
        autoMin: 100,
        autoMax: 140,
        autoStep: 2,
        autoEvery: 4,
        autoUnit: 'bars',
        autoLoop: true,
        autoReverse: false
    }));

    let currentProfile = 0;
    let totalTime = 0;
    let totalTimerID = null;

    function updateTotalTime() {
        const m = Math.floor(totalTime / 60);
        const s = totalTime % 60;
        document.getElementById('totalTime').textContent = `Total Time: ${m.toString().padStart(2,'0')}:${s.toString().padStart(2,'0')}`;
    }

    function loadProfile(index) {
        const p = profiles[index];
        if (!p || typeof p !== 'object' || p.bpm == null) {
            console.warn('Invalid profile data for index', index, p);
            // Reset to defaults or skip
            return;
        }
        bpmInput.value = p.bpm;
        bpmRange.value = p.bpm;
        beatsPerBarSel.value = p.beatsPerBar;
        beatUnitSel.value = p.beatUnit;
        drumsToggle.checked = p.drumsToggle;
        drumsVol.value = p.drumsVol;
        countToggle.checked = p.countToggle;
        autoToggle.checked = p.autoBpmToggle;
        autoMin.value = p.autoMin;
        autoMax.value = p.autoMax;
        autoStep.value = p.autoStep;
        autoEvery.value = p.autoEvery;
        autoUnit.value = p.autoUnit;
        autoLoop.checked = p.autoLoop;
        autoReverse.checked = p.autoReverse;
        updateAutoUI();
        renderBar();
    }

    function saveProfile(index) {
        if (!profiles[index]) {
            profiles[index] = {
                bpm: 120,
                beatsPerBar: 4,
                beatUnit: 4,
                drumsToggle: true,
                drumsVol: 0.8,
                countToggle: true,
                autoBpmToggle: false,
                autoMin: 100,
                autoMax: 140,
                autoStep: 2,
                autoEvery: 4,
                autoUnit: 'bars',
                autoLoop: true,
                autoReverse: false
            };
        }
        const p = profiles[index];
        p.bpm = Number(bpmInput.value);
        p.beatsPerBar = Number(beatsPerBarSel.value);
        p.beatUnit = Number(beatUnitSel.value);
        p.drumsToggle = drumsToggle.checked;
        p.drumsVol = Number(drumsVol.value);
        p.countToggle = countToggle.checked;
        p.autoBpmToggle = autoToggle.checked;
        p.autoMin = Number(autoMin.value);
        p.autoMax = Number(autoMax.value);
        p.autoStep = Number(autoStep.value);
        p.autoEvery = Number(autoEvery.value);
        p.autoUnit = autoUnit.value;
        p.autoLoop = autoLoop.checked;
        p.autoReverse = autoReverse.checked;
        localStorage.setItem('metronomeProfiles', JSON.stringify(profiles));
    }

    // Load from localStorage
    const saved = localStorage.getItem('metronomeProfiles');
    if (saved) {
        try {
            const parsed = JSON.parse(saved);
            if (Array.isArray(parsed) && parsed.length >= 5) {
                for (let i = 0; i < 5; i++) {
                    if (parsed[i] && typeof parsed[i] === 'object' && 'bpm' in parsed[i]) {
                        profiles[i] = parsed[i];
                    }
                }
            }
        } catch (e) {
            console.warn('Failed to load profiles', e);
        }
    }

    // Tab buttons
    document.querySelectorAll('.tab').forEach(btn => {
        btn.addEventListener('click', () => {
            saveProfile(currentProfile);
            const activeBtn = document.querySelector('.tab.active');
            if (activeBtn) activeBtn.classList.remove('active');
            btn.classList.add('active');
            currentProfile = Number(btn.dataset.profile);
            loadProfile(currentProfile);
        });
    });

    // === Audio ===
    let audioCtx=null, masterGain=null, clickGain=null, drumGain=null;
    let isRunning=false, currentBeat=0, nextNoteTime=0, timerID=null;

    const samples={kick:null,snare:null,hhc:null,hho:null,crash:null,tom:null};

    // Планировщик
    const lookahead=25; // ms
    const scheduleAhead=0.1; // sec

    // Состояния
    let patternAlt=0, barsSinceStart=0, lastBeatIndex=null, currentBar=1;
    let autoBarCounter=0, autoDir=1, autoElapsedSec=0, autoElapsedSecApprox=0, autoLastUIStamp=0;
    let runToken=0; let visualTimers=[]; let uiTimerID=null;
    let crashOnNextDownbeat=false;

    loadProfile(currentProfile);

    // === Семплы accelonome ===
    const SAMPLE_URLS = {
      kick: 'audio/kick.wav',
      snare: 'audio/snare.wav',
      hhc: 'audio/hihat-closed.wav',
      hho: 'audio/hihat-open.wav',
      crash: 'audio/crash.wav',
      tom: 'audio/tom.wav'
    };

    // === Паттерн барабанов из Accelonome ===
    const DRUMS_PATTERN={
      4:{
        4:{
          "open_hihat": { "inbetween": [[], [15]], "filler": [], "volume": 0.3, "duration": 0.4 },
          "closed_hithat": { "inbetween": [[3, 7, 11, 15], [3, 7, 11]], "volume": 0.05, "duration": 1 },
          "crash_cymbal": { "first_bar": [1], "inbetween": [], "volume": 0.5, "duration": 2 },
          "kick": { "inbetween": [[1, 9, 11]], "volume": 0.95, "duration": 1 },
          "snare": { "inbetween": [[5, 13]], "filler": [5, 13, 15, 16], "volume": 1, "duration": 1 }
        },
        5:{
          "closed_hithat": { "inbetween": [[1, 5, 9, 13, 17]], "volume": 0.05, "duration": 1 },
          "crash_cymbal": { "first_bar": [1], "inbetween": [], "volume": 0.5, "duration": 2 },
          "kick": { "inbetween": [[1]], "volume": 0.95, "duration": 1 },
          "snare": { "inbetween": [[13]], "volume": 0.8, "duration": 1 },
          // "tom": { "filler": [18], "inbetween": [], "volume": 0.9, "duration": 1 }
        },
        6:{
          "open_hihat": { "inbetween": [[], [11]], "filler": [], "volume": 0.3, "duration": 0.5 },
          "closed_hithat": { "inbetween": [[1,3,5,7,9,11,13,15,17,19], [1,3,5,7,9,11,13,15,17]], "volume": 0.05, "duration": 1 },
          "crash_cymbal": { "first_bar": [1], "inbetween": [], "volume": 0.5, "duration": 2 },
          "kick": { "inbetween": [[1]], "volume": 0.95, "duration": 1 },
          "snare": { "inbetween": [[7]], "filler": [7,9,10], "volume": 0.8, "duration": 1 },
          "tom": { "filler": [11], "inbetween": [], "volume": 1, "duration": 1 }
        }
      },
      8:{
        6:{
          "open_hihat": { "inbetween": [[], [11]], "filler": [], "volume": 0.3, "duration": 0.5 },
          "closed_hithat": { "inbetween": [[1,3,5,7,9,11,13,15,17,19], [1,3,5,7,9,11,13,15,17]], "volume": 0.05, "duration": 1 },
          "crash_cymbal": { "first_bar": [1], "inbetween": [], "volume": 0.5, "duration": 2 },
          "kick": { "inbetween": [[1]], "volume": 0.95, "duration": 1 },
          "snare": { "inbetween": [[7]], "filler": [7,9,10], "volume": 0.8, "duration": 1 },
          "tom": { "filler": [11], "inbetween": [], "volume": 1, "duration": 1 }
        }
      }
    };

    // Утилиты
    function bpm(){return Math.min(300,Math.max(20,Number(bpmInput.value)||120));}
    function spb(){ return (60/bpm())*(4/denom()); }
    function beatsPerBar(){return Number(beatsPerBarSel.value)||4;}
    function stepsPerBarForDenominator(den){ return den <= 8 ? 20 : 16; }

    function initAudio(){
      if(audioCtx) return;
      audioCtx=new (window.AudioContext||window.webkitAudioContext)();
      masterGain=audioCtx.createGain(); masterGain.gain.value=0.9; masterGain.connect(audioCtx.destination);
      clickGain=audioCtx.createGain(); clickGain.gain.value=0.6; clickGain.connect(masterGain);
      drumGain=audioCtx.createGain(); drumGain.gain.value=Number(drumsVol.value); drumGain.connect(masterGain);
    }

    // Загрузка сэмплов
    async function fetchAB(url){ const res=await fetch(url,{mode:'cors'}); if(!res.ok) throw new Error('HTTP '+res.status); return res.arrayBuffer(); }
    async function loadSample(url){ const ab=await fetchAB(url); return await audioCtx.decodeAudioData(ab.slice(0)); }
    async function loadSamples(){ initAudio(); for(const [k,u] of Object.entries(SAMPLE_URLS)){ try{ samples[k]=await loadSample(u);}catch(e){ samples[k]=null; console.warn('Не удалось загрузить',k,u,e);} } }

    // Метроно﻿м клик
    function clickAt(t, accented){ const o=audioCtx.createOscillator(); const g=audioCtx.createGain(); o.type='square'; o.frequency.setValueAtTime(accented?1600:1000,t); g.gain.setValueAtTime(0.0001,t); g.gain.exponentialRampToValueAtTime(accented?0.9:0.6,t+0.002); g.gain.exponentialRampToValueAtTime(0.0001,t+0.06); o.connect(g).connect(clickGain); o.start(t); o.stop(t+0.07); }

    // Воспроизведение сэмплов + фолбэки для хетов
    function playSample(buf, when, vol){ if(!buf) return; const src=audioCtx.createBufferSource(); src.buffer=buf; const g=audioCtx.createGain(); g.gain.setValueAtTime(Math.min(1,Math.max(0,vol)), when); src.connect(g).connect(drumGain); src.start(when); }
    function playKickSynth(t, v=0.9){ const osc=audioCtx.createOscillator(); const g=audioCtx.createGain(); osc.type='sine'; osc.frequency.setValueAtTime(120,t); osc.frequency.exponentialRampToValueAtTime(45,t+0.14); g.gain.setValueAtTime(v,t); g.gain.exponentialRampToValueAtTime(0.001,t+0.18); osc.connect(g).connect(drumGain); osc.start(t); osc.stop(t+0.18); }
    function playSnareSynth(t, v=1.0){ const len=Math.floor(audioCtx.sampleRate*0.12); const buf=audioCtx.createBuffer(1,len,audioCtx.sampleRate); const data=buf.getChannelData(0); for(let i=0;i<len;i++){ data[i]=(Math.random()*2-1)*Math.pow(1 - i/len, 2); } const src=audioCtx.createBufferSource(); src.buffer=buf; const hp=audioCtx.createBiquadFilter(); hp.type='highpass'; hp.frequency.value=1500; const bp=audioCtx.createBiquadFilter(); bp.type='bandpass'; bp.frequency.value=1800; bp.Q.value=0.8; const g=audioCtx.createGain(); g.gain.setValueAtTime(v,t); g.gain.exponentialRampToValueAtTime(0.001,t+0.12); src.connect(hp).connect(bp).connect(g).connect(drumGain); src.start(t); src.stop(t+0.12); }
    function playKick(t, v=0.9){ if(samples.kick){ playSample(samples.kick,t,v);} else { playKickSynth(t,v);} }
    function playSnare(t, v=1.0){ if(samples.snare){ playSample(samples.snare,t,v);} else { playSnareSynth(t,v);} }
    function synthNoiseHat(t, v, dur){ const src=audioCtx.createBufferSource(); const length=Math.floor(audioCtx.sampleRate*dur); const buf=audioCtx.createBuffer(1,length,audioCtx.sampleRate); const data=buf.getChannelData(0); for(let i=0;i<length;i++){ data[i]=Math.random()*2-1; } src.buffer=buf; const hp=audioCtx.createBiquadFilter(); hp.type='highpass'; hp.frequency.value=6000; const bp=audioCtx.createBiquadFilter(); bp.type='bandpass'; bp.frequency.value=9000; bp.Q.value=0.8; const g=audioCtx.createGain(); g.gain.setValueAtTime(v,t); g.gain.exponentialRampToValueAtTime(0.0001,t+dur); src.connect(hp).connect(bp).connect(g).connect(drumGain); src.start(t); }
    function playHatClosed(t, v=0.5){ if(samples.hhc){ playSample(samples.hhc,t,v);} else { synthNoiseHat(t,v,0.06);} }
    function playHatOpen(t, v=0.5){ if(samples.hho){ playSample(samples.hho,t,v);} else { synthNoiseHat(t,v,0.35);} }
    function playCrashSynth(t, v=0.5){ const len=Math.floor(audioCtx.sampleRate*0.25); const buf=audioCtx.createBuffer(1,len,audioCtx.sampleRate); const data=buf.getChannelData(0); for(let i=0;i<len;i++){ data[i]=(Math.random()*2-1)*Math.pow(1 - i/len, 3); } const src=audioCtx.createBufferSource(); src.buffer=buf; const hp=audioCtx.createBiquadFilter(); hp.type='highpass'; hp.frequency.value=800; const bp=audioCtx.createBiquadFilter(); bp.type='bandpass'; bp.frequency.value=2500; bp.Q.value=0.6; const g=audioCtx.createGain(); g.gain.setValueAtTime(v,t); g.gain.exponentialRampToValueAtTime(0.0001,t+0.4); src.connect(hp).connect(bp).connect(g).connect(drumGain); src.start(t); src.stop(t+0.4); }
    function playCrash(t, v=0.5){ if(samples.crash){ playSample(samples.crash,t,v);} else { playCrashSynth(t,v);} }
    function playTomSynth(t, v=0.8){ const osc=audioCtx.createOscillator(); osc.type='triangle'; osc.frequency.setValueAtTime(220,t); osc.frequency.exponentialRampToValueAtTime(100,t+0.18); const g=audioCtx.createGain(); g.gain.setValueAtTime(v,t); g.gain.exponentialRampToValueAtTime(0.001,t+0.25); osc.connect(g).connect(drumGain); osc.start(t); osc.stop(t+0.28); }
    function playTom(t, v=0.8){ if(samples.tom){ playSample(samples.tom,t,v);} else { playTomSynth(t,v);} }

    // Плейер паттернов; filler в последнем такте перед изменением
    function denom(){ return Number(beatUnitSel.value)||4; }
    function scheduleDrumsFromPattern(beatIndex, t){
      if(!drumsToggle.checked) return;
      const den=denom(); const num=beatsPerBar(); const byDen=DRUMS_PATTERN[den]; if(!byDen) return; const pat=byDen[num]; if(!pat) return;
      const stepsTotal=stepsPerBarForDenominator(den); const barLen=num*spb(); const stepDur=barLen/stepsTotal; const barStart=t - beatIndex*spb();
      const from=Math.floor(beatIndex*(stepsTotal/num)); const toEx=Math.floor((beatIndex+1)*(stepsTotal/num));

      function shouldPlayFiller(){ const cfg=autoCfg(); if(!cfg.enabled) return false; const lastBeat=beatIndex===num-1; if(!lastBeat) return false; if(cfg.unit==='bars'){ const barsNeed=(cfg.barsV||1); return autoBarCounter===barsNeed-1; } else { const need=(cfg.minutesV||1)*60; const remaining=Math.max(0,need-autoElapsedSec); return remaining <= num*spb() + 1e-6; } }
      const playFiller = shouldPlayFiller();

      let trigger=()=>{};
      function scheduleList(list, v, d){ if(!Array.isArray(list)) return; for(const idx of list){ const s=(idx-1); if(s>=from && s<toEx){ const when=t; trigger(when,v,d); } } }

      for(const name in pat){
        const cfgI=pat[name]; const v=Number(cfgI.volume||1); const d=Number(cfgI.duration||0.2);
        if(name==='kick') trigger=(w)=>playKick(w,v);
        else if(name==='snare') trigger=(w)=>playSnare(w,v);
        else if(name==='closed_hihat' || name==='closed_hithat') trigger=(w)=>playHatClosed(w,v);
        else if(name==='open_hihat') trigger=(w)=>playHatOpen(w,v);
        else if(name==='crash_cymbal') trigger=(w)=>playCrash(w,v);
        else if(name==='tom') trigger=(w)=>playTom(w,v);
        else trigger=()=>{};

        if(name==='crash_cymbal'){
          if(barsSinceStart===0) scheduleList([1], v, d);
        } else {
          if(cfgI.first_bar && barsSinceStart===0) scheduleList(cfgI.first_bar,v,d);
          if(Array.isArray(cfgI.inbetween)){ const alt = cfgI.inbetween[Math.min(patternAlt, cfgI.inbetween.length-1)] || []; scheduleList(alt,v,d); }
          if(playFiller && Array.isArray(cfgI.filler) && cfgI.filler.length){ scheduleList(cfgI.filler, v, d); }
        }
      }
    }

    // Schedule drums for the bar, like in Accelonome
    function scheduleDrums(t){
      if(!drumsToggle.checked) return;
      const den = denom();
      const num = beatsPerBar();
      const pat = DRUMS_PATTERN[den] && DRUMS_PATTERN[den][num];
      if(!pat) return;
      
      // Check if we should play filler (last bar before BPM change)
      let shouldPlayFiller = false;
      const cfg = autoCfg();
      if(cfg.enabled && currentBar > 1) {
        if(cfg.unit === 'bars') {
          const barsNeed = cfg.barsV || 1;
          shouldPlayFiller = autoBarCounter === barsNeed - 1;
        } else {
          const need = (cfg.minutesV || 1) * 60;
          const remaining = Math.max(0, need - autoElapsedSec);
          shouldPlayFiller = remaining <= num * spb() + 1e-6;
        }
      }
      
      const sixteenthTime = 60.0 / (bpm() * 4);
      for (const [instrument, config] of Object.entries(pat)) {
        let beat = null;
        if (currentBar == 1 && 'first_bar' in config) {
          beat = config['first_bar'];
        } else if (shouldPlayFiller && 'filler' in config) {
          beat = config['filler'];
        } else if (config['inbetween'].length >= 1) {
          beat = config['inbetween'][0];
        } else {
          continue;
        }
        for (const sixteenthNote of beat) {
          const volume = config['volume'];
          const when = t + (sixteenthNote - 1) * sixteenthTime;
          if(instrument === 'kick') playKick(when, volume);
          else if(instrument === 'snare') playSnare(when, volume);
          else if(instrument === 'closed_hithat' || instrument === 'closed_hihat') playHatClosed(when, volume);
          else if(instrument === 'open_hihat') playHatOpen(when, volume);
          else if(instrument === 'crash_cymbal') playCrash(when, volume);
          else if(instrument === 'tom') playTom(when, volume);
        }
      }
      
      // Play crash on first beat if tempo just changed
      if(drumsToggle.checked && crashOnNextDownbeat) {
        playCrash(t, 0.5);
        crashOnNextDownbeat = false;
      }
    }

    // Визуализатор
    function renderBar(){ bar.innerHTML=''; const n=beatsPerBar(); bar.style.gridTemplateColumns='repeat('+n+',1fr)'; for(let i=0;i<n;i++){ const d=document.createElement('div'); d.className='seg'; bar.appendChild(d); } if(progressFill){ progressFill.style.transition='none'; progressFill.style.width='0%'; } }
    function updateBar(idx){ const n=bar.children.length||1; const stepPct=100/n; const targetPct=Math.min(100,(idx+1)*stepPct); const dur=spb(); if(progressFill){ progressFill.style.transition='width '+dur+'s linear'; if(idx===0){ progressFill.style.transition='none'; progressFill.style.width='0%'; requestAnimationFrame(()=>{ progressFill.style.transition='width '+dur+'s linear'; progressFill.style.width=stepPct+'%'; }); } else { progressFill.style.width=targetPct+'%'; } } }
    function clearVisuals(){ if(progressFill){ progressFill.style.transition='none'; progressFill.style.width='0%'; } visualTimers.forEach(clearTimeout); visualTimers=[]; }
    function scheduleVisual(beatIndex,time){ if(!audioCtx) return; const delay=Math.max(0,time-audioCtx.currentTime); const token=runToken; const id=setTimeout(()=>{ if(!isRunning) return; if(token!==runToken) return; updateBar(beatIndex); },delay*1000); visualTimers.push(id); }

    // Auto‑BPM
    function updateBarsCounterUI(){ const cfg=autoCfg(); if(!cfg.enabled || cfg.unit!=='bars'){ return; } const total = cfg.barsV||1; const cur = Math.min(total, (autoBarCounter||0) + 1); nextStepTimer.textContent = cur + '/' + total; }
    function autoCfg(){ const en=autoToggle.checked; const minV=Math.min(300,Math.max(20,Number(autoMin.value)||100)); const maxV=Math.min(300,Math.max(20,Number(autoMax.value)||140)); const stepV=Math.max(1,Math.floor(Number(autoStep.value)||1)); const unit=autoUnit.value; const each=Number(autoEvery.value)||4; const barsV=unit==='bars'?Math.max(1,Math.floor(each)):null; const minutesV=unit==='minutes'?Math.max(0.1,each):null; const loopV=autoLoop.checked; const reverse=autoReverse.checked; return {enabled:en,minV:Math.min(minV,maxV),maxV:Math.max(minV,maxV),stepV,unit,barsV,minutesV,loopV,reverse}; }
    function applyBpm(v){ v=Math.round(Math.min(300,Math.max(20,v))); bpmInput.value=v; bpmRange.value=v; }
    function updateAutoUI(){ const en=autoToggle.checked; if(autoGroup) autoGroup.classList.toggle('disabled',!en); [autoMin,autoMax,autoStep,autoEvery,autoUnit,autoLoop,autoReverse,autoResetBtn].forEach(el=>el.disabled=!en); const cfg=autoCfg(); if(!en){ nextStepTimer.style.visibility='hidden'; nextStepTimer.textContent='—:—'; return; } if(cfg.unit==='minutes'){ nextStepTimer.style.visibility='visible'; updateTimerUI(); } else { nextStepTimer.style.visibility='visible'; updateBarsCounterUI(); } }

    function fmtTime(sec){ sec=Math.max(0,sec); const m=Math.floor(sec/60); const s=Math.floor(sec%60); return m+":"+String(s).padStart(2,'0'); }
    function updateTimerUI(){ const cfg=autoCfg(); if(!isRunning||!cfg.enabled||cfg.unit!=='minutes')return; const need=(cfg.minutesV||1)*60; const extra=Math.max(0,(performance.now()-autoLastUIStamp)/1000); const elapsed=autoElapsedSecApprox+extra; const remain=Math.max(0,need-elapsed); nextStepTimer.textContent=fmtTime(remain); }
    function startUITimer(){ if(uiTimerID) clearInterval(uiTimerID); uiTimerID=setInterval(()=>{ const cfg=autoCfg(); if(cfg.enabled && cfg.unit==='minutes'){ updateTimerUI(); } },100); const cfg=autoCfg(); if(cfg.enabled && cfg.unit==='minutes'){ updateTimerUI(); } }
    function stopUITimer(){ if(uiTimerID){ clearInterval(uiTimerID); uiTimerID=null; } }

    function bumpBpmAccordingTo(cfg){
      const cur=bpm();
      if(cfg.reverse){
        let next=cur + cfg.stepV*autoDir;
        if(next>cfg.maxV){ const over=next-cfg.maxV; autoDir=-1; next=Math.max(cfg.minV, cfg.maxV-over); }
        else if(next<cfg.minV){ const over=cfg.minV-next; autoDir=1; next=Math.min(cfg.maxV, cfg.minV+over); }
        applyBpm(next);
      } else {
        let next=cur + cfg.stepV; if(next>cfg.maxV) next = cfg.loopV ? cfg.minV : cfg.maxV; if(next<cfg.minV) next = cfg.minV; applyBpm(next);
      }
      crashOnNextDownbeat = true; // включить Crash на следующей первой доле
    }

    function maybeAutoAdvance(beatIndex){
      const cfg=autoCfg(); if(!cfg.enabled){ lastBeatIndex=beatIndex; return; }
      const secPerBeat=spb();
      if(cfg.unit==='minutes'){
        autoElapsedSec += secPerBeat; autoElapsedSecApprox += secPerBeat; autoLastUIStamp = performance.now();
        const need=(cfg.minutesV||1)*60;
        if(autoElapsedSec + 1e-6 >= need){ autoElapsedSec=0; autoElapsedSecApprox=0; autoLastUIStamp=performance.now(); bumpBpmAccordingTo(cfg); }
      } else { // bars
        if(lastBeatIndex===null){ lastBeatIndex=beatIndex; return; }
        if(beatIndex===0 && lastBeatIndex!==0){ autoBarCounter++; const barsNeed=(cfg.barsV||1); if(autoBarCounter>=barsNeed){ autoBarCounter=0; bumpBpmAccordingTo(cfg); } updateBarsCounterUI(); }
      }
      lastBeatIndex=beatIndex;
    }

    // Планировщик аудио
    function nextNote(){ nextNoteTime += spb(); currentBeat = (currentBeat+1) % beatsPerBar(); if(currentBeat===0){ barsSinceStart++; patternAlt=1-patternAlt; currentBar++; } }
    function scheduler(){
      while(nextNoteTime < audioCtx.currentTime + scheduleAhead){
        const accented = currentBeat===0;
        clickAt(nextNoteTime, accented);
        // ВАЖНО: сначала обновляем счётчик тактов/времени
        maybeAutoAdvance(currentBeat);
        // Паттерны и визуал планируем с обновленным autoBarCounter
        if(currentBeat === 0) scheduleDrums(nextNoteTime);
        scheduleVisual(currentBeat, nextNoteTime);
        nextNote();
      }
      timerID = setTimeout(scheduler, lookahead);
    }

    // Countdown function
    async function countdown() {
        initAudio();
        try { await audioCtx.resume(); } catch {}
        countdownDiv.style.display = 'block';
        for (let i = 3; i > 0; i--) {
            countdownDiv.textContent = i;
            clickAt(audioCtx.currentTime, true); // accented click for countdown
            await new Promise(resolve => setTimeout(resolve, 1000));
        }
        countdownDiv.style.display = 'none';
    }

    // Контролы
    async function start(){
        saveProfile(currentProfile); // Save current profile before starting
        if (countToggle.checked) {
            await countdown();
        }
        if(isRunning) return;
        isRunning=true; autoBarCounter=0; autoDir=1; lastBeatIndex=null; autoElapsedSec=0; autoElapsedSecApprox=0; autoLastUIStamp=performance.now(); barsSinceStart=0; patternAlt=0; currentBar=1; crashOnNextDownbeat=false; const cfg=autoCfg(); if(cfg.enabled && (bpm()<cfg.minV || bpm()>cfg.maxV)) applyBpm(cfg.minV); runToken++; clearVisuals(); renderBar(); currentBeat=0; nextNoteTime = audioCtx.currentTime; if(audioCtx && drumGain) drumGain.gain.setValueAtTime(Number(drumsVol.value), audioCtx.currentTime); scheduler(); startBtn.disabled=true; stopBtn.disabled=false; startUITimer(); updateAutoUI(); updateBarsCounterUI();
        totalTimerID = setInterval(() => { totalTime++; updateTotalTime(); }, 1000);
    }
    function stop(){ if(!isRunning) return; isRunning=false; if(timerID){clearTimeout(timerID); timerID=null;} startBtn.disabled=false; stopBtn.disabled=true; currentBeat=0; currentBar=1; runToken++; clearVisuals(); stopUITimer(); nextStepTimer.textContent='—:—'; nextStepTimer.style.visibility='hidden'; if(totalTimerID){ clearInterval(totalTimerID); totalTimerID=null; } if(audioCtx && drumGain && audioCtx.currentTime) drumGain.gain.setValueAtTime(0, audioCtx.currentTime); }

    // Привязки UI
    bpmRange.addEventListener('input',e=>{ bpmInput.value=e.target.value; });
    bpmInput.addEventListener('input',e=>{ bpmRange.value=e.target.value; });
    beatsPerBarSel.addEventListener('change',()=>{ currentBeat=0; renderBar(); clearVisuals(); });
    beatUnitSel.addEventListener('change',()=>{ currentBeat=0; renderBar(); clearVisuals(); });
    drumsVol.addEventListener('input',()=>{ if(audioCtx) drumGain.gain.value=Number(drumsVol.value); });
    startBtn.addEventListener('click',start);
    stopBtn.addEventListener('click',stop);

    autoToggle.addEventListener('change',updateAutoUI);
    ;[autoMin,autoMax,autoStep,autoEvery,autoUnit,autoLoop,autoReverse].forEach(el=>el.addEventListener('input',updateAutoUI));
    autoResetBtn.addEventListener('click',()=>{ const cfg=autoCfg(); applyBpm(cfg.minV); autoBarCounter=0; lastBeatIndex=null; autoDir=1; autoElapsedSec=0; autoElapsedSecApprox=0; autoLastUIStamp=performance.now(); crashOnNextDownbeat=false; updateAutoUI(); updateBarsCounterUI(); });

    // Автозагрузка семплов
    (async()=>{ try{ await loadSamples(); }catch(e){ console.warn('Ошибка загрузки семплов', e); } })();

    // Инициализация UI
    updateAutoUI(); renderBar();

    // Самотесты
    try{
      console.group('%cSelf-tests','color:#22c55e');
      ['startBtn','stopBtn','bpmInput','bpmRange','beatsPerBar','beatUnit','progressFill','bar','autoBpmToggle','autoMin','autoMax','autoStep','autoEvery','autoUnit','autoLoop','autoReverse','autoResetBtn','nextStepTimer'].forEach(id=>console.assert(document.getElementById(id),'нет '+id));
      const prev=bpm(); bpmInput.value=prev+1; bpmInput.dispatchEvent(new Event('input')); console.assert(Number(bpmRange.value)===prev+1,'bpmInput->bpmRange'); bpmRange.value=prev; bpmRange.dispatchEvent(new Event('input')); console.assert(Number(bpmInput.value)===prev,'bpmRange->bpmInput');
      console.groupEnd();
    }catch(e){ console.error('Self-tests error', e); }

    // Keyboard controls
    document.addEventListener('keydown', (e) => {
        if (e.code === 'Space') {
            e.preventDefault(); // Prevent page scroll
            if (isRunning) {
                stop();
            } else {
                start();
            }
        }
    });
  });
