#!/usr/bin/env python3
"""Generates GRIN.amxd (Max for Live audio effect) + GRIN.maxpat + grin.genexpr.
The DSP is a gen~ codebox that mirrors Source/GrinEngine.h."""
import json, struct, os

HERE = os.path.dirname(os.path.abspath(__file__))

# ---------------------------------------------------------------- GenExpr --
FUNCS = r'''
// ---------- helpers (each call site gets its own state) ----------
g_sstep(e0, e1, x) {
	t = clamp((x - e0) / (e1 - e0), 0, 1);
	return t * t * (3 - 2 * t);
}

g_pick(m, a0, a1, a2, a3, a4, a5, a6) {
	return m < 0.5 ? a0 : (m < 1.5 ? a1 : (m < 2.5 ? a2 : (m < 3.5 ? a3 : (m < 4.5 ? a4 : (m < 5.5 ? a5 : a6)))));
}

g_slew(x, c) {
	History y(0);
	History started(0);
	v = started > 0.5 ? y + c * (x - y) : x;
	started = 1;
	y = v;
	return v;
}

// TPT state-variable filter, highpass output
g_hp(x, fc, q, sr) {
	History ic1(0);
	History ic2(0);
	g = tan(3.14159265 * fc / sr);
	k = 1 / q;
	a1 = 1 / (1 + g * (g + k));
	a2 = g * a1;
	a3 = g * a2;
	v3 = x - ic2;
	v1 = a1 * ic1 + a2 * v3;
	v2 = ic2 + a2 * ic1 + a3 * v3;
	ic1 = 2 * v1 - ic1;
	ic2 = 2 * v2 - ic2;
	return x - k * v1 - v2;
}

// TPT state-variable filter, bandpass output
g_bp(x, fc, q, sr) {
	History ic1(0);
	History ic2(0);
	g = tan(3.14159265 * fc / sr);
	k = 1 / q;
	a1 = 1 / (1 + g * (g + k));
	a2 = g * a1;
	a3 = g * a2;
	v3 = x - ic2;
	v1 = a1 * ic1 + a2 * v3;
	v2 = ic2 + a2 * ic1 + a3 * v3;
	ic1 = 2 * v1 - ic1;
	ic2 = 2 * v2 - ic2;
	return v1;
}

g_flange(x, lfo, depthms, fb, amt, sr) {
	Delay d(8192);
	tap = d.read((0.6 + depthms * lfo) * 0.001 * sr);
	d.write(x + fb * tap);
	return x + amt * (0.5 * (x + tap) - x);
}

g_pshift(x, ph, win, amt) {
	Delay d(16384);
	p2 = ph + 0.5;
	p2 = p2 - floor(p2);
	w1 = sin(3.14159265 * ph);
	w2 = sin(3.14159265 * p2);
	y = d.read(1 + (1 - ph) * win) * w1 * w1 + d.read(1 + (1 - p2) * win) * w2 * w2;
	d.write(x);
	return x + amt * (y - x);
}

// tuned feedback comb with a lowpass in the loop (tonal riser)
g_comb(x, dsamps, fb, damp) {
	Delay d(8192);
	History lp(0);
	o = d.read(dsamps);
	nlp = lp + damp * (o - lp);
	lp = nlp;
	d.write(x + fb * nlp);
	return o;
}

// freeverb-style lowpass comb and allpass
g_fvcomb(x, len, fb, damp) {
	Delay d(8192);
	History f(0);
	o = d.read(len);
	nf = o * (1 - damp) + f * damp;
	f = nf;
	d.write(x + nf * fb);
	return o;
}

g_allp(x, len) {
	Delay d(4096);
	b = d.read(len);
	d.write(x + b * 0.5);
	return b - x;
}

g_saw(freq, sr) {
	History ph(0);
	dt = freq / sr;
	p = ph + dt;
	p = p - floor(p);
	ph = p;
	s = 2 * p - 1;
	t = 0;
	if (p < dt) {
		t = p / dt;
		s = s - (t + t - t * t - 1);
	}
	else if (p > 1 - dt) {
		t = (p - 1) / dt;
		s = s - (t * t + t + t + 1);
	}
	return s;
}
'''

DECLS = r'''
// ---------- GRIN main ----------
// in1/in2 = audio, in3 = phasor~ 1n @lock 1 (one ramp per bar from Live's transport)
Param smile(0, min=0, max=1);
Param strength(2, min=0, max=6);
Param gateon(1, min=0, max=1);
Param noiseon(1, min=0, max=1);
History sm(0);
History env(0);
History lfoph(0);
History pph(0);
History gateenv(1);
'''

MAIN_TOP = r'''
sr = samplerate;

// smoothed knob
cs = 1 - exp(-1 / (0.02 * sr));
s = sm + cs * (smile - sm);
sm = s;

// strength mode table, glided so switching never clicks
m = floor(strength + 0.5);
gl = 0.002;
mDrive  = g_slew(g_pick(m, 0.30, 0.45, 0.65, 0.80, 0.95, 1.10, 1.40), gl);
mTop    = g_slew(g_pick(m, 3000, 3600, 4500, 5500, 6500, 7500, 9000), gl);
mRes    = g_slew(g_pick(m, 0.5, 0.6, 0.8, 1.0, 1.1, 1.2, 1.3), gl);
mFlange = g_slew(g_pick(m, 0.35, 0.5, 0.7, 0.85, 1.0, 1.1, 1.2), gl);
mNoise  = g_slew(g_pick(m, 0.55, 0.7, 0.85, 1.0, 1.05, 1.1, 1.2), gl);
mGate   = g_slew(g_pick(m, 0, 0.5, 0.7, 0.85, 0.95, 1, 1), gl);
mPitch  = g_slew(g_pick(m, 0, 0, 0.5, 0.8, 1, 1, 1.15), gl);
mSpace  = g_slew(g_pick(m, 0.75, 0.85, 0.9, 1, 1, 1.05, 1.1), gl);
mTone   = g_slew(g_pick(m, 0, 0, 0, 0, 0.6, 0.85, 1), gl);
mSynth  = g_slew(g_pick(m, 0, 0, 0, 0, 0, 0, 1), gl);

// knob -> stage curves (same as GrinEngine::updateCoefficients)
driveMix  = clamp(s * 4, 0, 1);
driveAmt  = 1 + 7 * mDrive * s * s;
driveNorm = 1 / sqrt(driveAmt);

hpMix = clamp(s * 8, 0, 1);
hpCut = min(sr * 0.45, 20 * pow(mTop / 20, pow(s, 1.5)));
hpQ2  = 1.3 + 1.4 * mRes * s * s;

flMix   = min(0.8, 0.8 * mFlange * g_sstep(0.15, 0.75, s));
flRate  = 0.12 + 2.8 * s * s;
flDepth = 0.8 + 3.2 * s;
flFb    = min(0.85, 0.75 * s * mFlange);

nT     = g_sstep(0.1, 1, s);
nLevel = noiseon > 0.5 ? 0.24 * mNoise * pow(nT, 1.5) : 0;
nCut   = min(sr * 0.45, 300 * pow(40, s));
nQ     = 1.1 + 1.5 * s;

tT         = g_sstep(0.2, 1, s);
toneLevel  = mTone * pow(tT, 1.2);
synthLevel = mSynth * pow(g_sstep(0.35, 1, s), 1.5);
toneFreq   = 130.81 * pow(2, 3 * tT);
toneFb     = 0.92 + 0.06 * tT;
toneNorm   = 3.2 * (1 - toneFb) / (1 - toneFb * 0.9);

gDepth = 0;
gDiv = 1;
gfrac = 0;
if (gateon > 0.5 && mGate > 0.01 && s >= 0.35) {
	gfrac = (s - 0.35) / 0.65;
	gDepth = clamp((s - 0.35) / 0.05, 0, 1) * (0.3 + 0.55 * gfrac) * mGate;
	gDiv = s < 0.6 ? 1 : (s < 0.8 ? 0.5 : (s < 0.93 ? 0.25 : 0.125));
}

pT    = g_sstep(0.55, 1, s);
pMix  = 0.35 * pT * mPitch;
pWin  = 0.045 * sr;
pStep = (pow(2, pT) - 1) / pWin;

revWet = 0.22 * mSpace * pow(s, 1.3);
dryG   = 1 - 0.3 * s;
room   = min(0.98, 0.45 + 0.53 * s * mSpace);
fvFb   = room * 0.28 + 0.7;
fvDamp = (0.55 - 0.4 * s) * 0.4;

side    = 1 + 0.7 * s;
outG    = pow(10, 9 * pow(s, 1.5) / 20);
clipMix = clamp(s * 5, 0, 1);

// ---------- per-sample modulators ----------
inL = in1;
inR = in2;
lvl = 0.5 * (abs(inL) + abs(inR));
ec = lvl > env ? exp(-1 / (0.005 * sr)) : exp(-1 / (0.3 * sr));
envNow = lvl + ec * (env - lvl);
env = envNow;
follow = min(1, envNow * 4);

lp = lfoph + flRate / sr;
lp = lp - floor(lp);
lfoph = lp;
lfoL = 0.5 + 0.5 * sin(6.2831853 * lp);
lpR = lp + 0.25;
lpR = lpR - floor(lpR);
lfoR = 0.5 + 0.5 * sin(6.2831853 * lpR);

pp = pph + pStep;
pp = pp - floor(pp);
pph = pp;

gph = in3 * 4 / gDiv;
gph = gph - floor(gph);
gTarget = gDepth > 0 ? (gph < 0.55 ? 1 : 0) : 1;
gc = exp(-1 / (0.0015 * sr));
ge = gTarget + gc * (gateenv - gTarget);
gateenv = ge;
gGain = 1 - gDepth * (1 - ge);

// synth saw riser (Extreme)
sw1 = g_saw(toneFreq * 0.993, sr);
sw2 = g_saw(toneFreq, sr);
sw3 = g_saw(toneFreq * 1.007, sr);
synL = (sw1 + sw2 + sw3 * 0.6) * 0.12 * synthLevel * follow;
synR = (sw1 * 0.6 + sw2 + sw3) * 0.12 * synthLevel * follow;
'''

CHAN = r'''
// ---------- channel {C} ----------
x{C} = in{N};
x{C} = x{C} + driveMix * (tanh(x{C} * driveAmt) * driveNorm - x{C});
h{C} = g_hp(g_hp(x{C}, hpCut, 0.54, sr), hpCut, hpQ2, sr);
x{C} = x{C} + hpMix * (h{C} - x{C});
x{C} = g_flange(x{C}, lfo{C}, flDepth, flFb, flMix, sr);
x{C} = x{C} + g_bp(noise(), nCut, nQ, sr) * nLevel * follow;
x{C} = g_pshift(x{C}, pp, pWin, pMix);
ex{C} = x{C} + 0.03 * noise() * follow;
ring{C} = g_comb(ex{C}, sr / (toneFreq * {SP}), toneFb, 0.55)
       + g_comb(ex{C}, sr / (toneFreq * 1.5 * {SP}), toneFb, 0.55) * 0.7
       + g_comb(ex{C}, sr / (toneFreq * 2 * {SP}), toneFb, 0.55) * 0.5;
x{C} = x{C} + ring{C} * toneNorm * toneLevel * follow + syn{C};
x{C} = x{C} * gGain;
'''

MAIN_END = r'''
// ---------- reverb (4 combs + 2 allpasses per side) ----------
k = sr / 44100;
rin = (xL + xR) * 0.015;
rL = g_fvcomb(rin, 1116 * k, fvFb, fvDamp) + g_fvcomb(rin, 1188 * k, fvFb, fvDamp)
   + g_fvcomb(rin, 1277 * k, fvFb, fvDamp) + g_fvcomb(rin, 1356 * k, fvFb, fvDamp);
rL = g_allp(g_allp(rL, 556 * k), 441 * k);
rR = g_fvcomb(rin, 1139 * k, fvFb, fvDamp) + g_fvcomb(rin, 1211 * k, fvFb, fvDamp)
   + g_fvcomb(rin, 1300 * k, fvFb, fvDamp) + g_fvcomb(rin, 1379 * k, fvFb, fvDamp);
rR = g_allp(g_allp(rR, 579 * k), 464 * k);

oL = xL * dryG + rL * revWet * 6;
oR = xR * dryG + rR * revWet * 6;

// ---------- width, makeup, safety clip ----------
mid = 0.5 * (oL + oR);
sd = 0.5 * (oL - oR) * side;
oL = (mid + sd) * outG;
oR = (mid - sd) * outG;
oL = oL + clipMix * (tanh(oL) - oL);
oR = oR + clipMix * (tanh(oR) - oR);

out1 = oL;
out2 = oR;
'''

code = (FUNCS + DECLS + MAIN_TOP
        + CHAN.replace('{C}', 'L').replace('{N}', '1').replace('{SP}', '1')
        + CHAN.replace('{C}', 'R').replace('{N}', '2').replace('{SP}', '1.003')
        + MAIN_END).strip() + '\n'

with open(os.path.join(HERE, 'grin.genexpr'), 'w') as f:
    f.write(code)

# ---------------------------------------------------------------- patcher --
APPVERSION = {"major": 8, "minor": 6, "revision": 2, "architecture": "x64", "modernui": 1}
FONT = "Ableton Sans Bold"

def box(id_, maxclass, rect, **kw):
    b = {"id": id_, "maxclass": maxclass, "patching_rect": rect}
    b.update(kw)
    return {"box": b}

def newobj(id_, text, rect, nin, nout, otypes):
    return box(id_, "newobj", rect, text=text, numinlets=nin, numoutlets=nout, outlettype=otypes)

def line(src, so, dst, di):
    return {"patchline": {"source": [src, so], "destination": [dst, di]}}

def param(longname, shortname, ptype, mmax, initial, unitstyle=None, enum=None):
    v = {"parameter_longname": longname, "parameter_shortname": shortname, "parameter_type": ptype,
         "parameter_mmax": mmax, "parameter_initial_enable": 1, "parameter_initial": [initial]}
    if unitstyle is not None: v["parameter_unitstyle"] = unitstyle
    if enum is not None: v["parameter_enum"] = enum
    return {"saved_attribute_attributes": {"valueof": v}, "parameter_enable": 1, "varname": longname}

gen_patcher = {
    "fileversion": 1, "appversion": APPVERSION, "classnamespace": "dsp.gen",
    "rect": [100, 100, 900, 700], "gridsize": [15, 15],
    "boxes": [
        newobj("g-in1", "in 1", [20, 20, 30, 22], 0, 1, [""]),
        newobj("g-in2", "in 2", [200, 20, 30, 22], 0, 1, [""]),
        newobj("g-in3", "in 3", [380, 20, 30, 22], 0, 1, [""]),
        box("g-code", "codebox", [20, 60, 800, 560], code=code, fontface=0, fontname="<Monospaced>",
            fontsize=12.0, numinlets=3, numoutlets=2, outlettype=["", ""]),
        newobj("g-out1", "out 1", [20, 640, 37, 22], 1, 0, []),
        newobj("g-out2", "out 2", [400, 640, 37, 22], 1, 0, []),
    ],
    "lines": [
        line("g-in1", 0, "g-code", 0), line("g-in2", 0, "g-code", 1), line("g-in3", 0, "g-code", 2),
        line("g-code", 0, "g-out1", 0), line("g-code", 1, "g-out2", 0),
    ],
}

PINK = [1.0, 0.243, 0.647, 1.0]
CYAN = [0.243, 0.902, 1.0, 1.0]
DIM  = [0.54, 0.52, 0.6, 1.0]
BG   = [0.051, 0.047, 0.071, 1.0]

boxes = [
    # background + labels (presentation)
    box("bg", "panel", [600, 20, 360, 169], presentation=1, presentation_rect=[0, 0, 360, 169],
        bgcolor=BG, mode=0, border=0, background=1, numinlets=1, numoutlets=0, ignoreclick=1),
    box("title", "comment", [600, 200, 80, 30], text="GRIN", presentation=1,
        presentation_rect=[10, 4, 80, 30], fontname=FONT, fontsize=22.0, textcolor=[0.95, 0.94, 0.97, 1.0],
        numinlets=1, numoutlets=0),
    box("sub", "comment", [700, 200, 150, 18], text="ONE-KNOB BUILD-UP", presentation=1,
        presentation_rect=[84, 14, 150, 18], fontname=FONT, fontsize=9.0, textcolor=DIM,
        numinlets=1, numoutlets=0),
    box("lblmode", "comment", [600, 240, 200, 18], text="STRENGTH", presentation=1,
        presentation_rect=[128, 44, 120, 18], fontname=FONT, fontsize=9.0, textcolor=DIM,
        numinlets=1, numoutlets=0),
    box("legend", "comment", [600, 260, 230, 18], text="S = Soft   M = Medium   H = Hard   EXT = Extreme",
        presentation=1, presentation_rect=[128, 88, 230, 18], fontname=FONT, fontsize=8.0, textcolor=DIM,
        numinlets=1, numoutlets=0),

    # controls
    box("dial", "live.dial", [20, 20, 90, 100], presentation=1, presentation_rect=[6, 40, 110, 122],
        numinlets=1, numoutlets=2, outlettype=["", "float"],
        showname=1, shownumber=1, fontname=FONT, fontsize=10.0,
        activedialcolor=PINK, activeneedlecolor=CYAN, dialcolor=[0.14, 0.13, 0.18, 1.0],
        textcolor=[0.95, 0.94, 0.97, 1.0],
        **param("Smile", "Smile", 0, 100.0, 0.0, unitstyle=5)),
    box("tab", "live.tab", [140, 20, 230, 22], presentation=1, presentation_rect=[128, 62, 226, 22],
        numinlets=1, numoutlets=3, outlettype=["", "", "float"], fontname=FONT, fontsize=9.0,
        activebgoncolor=PINK, num_lines_patching=1, num_lines_presentation=1,
        **param("Strength", "Strength", 2, 6.0, 2.0, enum=["S1", "S2", "M1", "M2", "H1", "H2", "EXT"])),
    box("gatebtn", "live.text", [140, 60, 60, 20], presentation=1, presentation_rect=[128, 120, 70, 24],
        numinlets=1, numoutlets=2, outlettype=["", ""], text="GATE", texton="GATE",
        activebgoncolor=PINK, fontname=FONT, fontsize=10.0,
        **param("Gate", "Gate", 2, 1.0, 1.0, enum=["off", "on"])),
    box("noisebtn", "live.text", [220, 60, 60, 20], presentation=1, presentation_rect=[206, 120, 70, 24],
        numinlets=1, numoutlets=2, outlettype=["", ""], text="NOISE", texton="NOISE",
        activebgoncolor=PINK, fontname=FONT, fontsize=10.0,
        **param("Noise", "Noise", 2, 1.0, 1.0, enum=["off", "on"])),

    # parameter -> gen~ messages
    newobj("scale", "* 0.01", [20, 140, 50, 22], 2, 1, ["float"]),
    newobj("pSmile", "prepend smile", [20, 170, 85, 22], 1, 1, [""]),
    newobj("pStrength", "prepend strength", [140, 170, 100, 22], 1, 1, [""]),
    newobj("pGate", "prepend gateon", [260, 170, 90, 22], 1, 1, [""]),
    newobj("pNoise", "prepend noiseon", [370, 170, 95, 22], 1, 1, [""]),

    # audio
    newobj("plugin", "plugin~", [20, 230, 60, 22], 2, 2, ["signal", "signal"]),
    newobj("phasor", "phasor~ 1n @lock 1", [300, 230, 120, 22], 2, 1, ["signal"]),
    box("gen", "newobj", [20, 280, 300, 22], text="gen~", numinlets=3, numoutlets=2,
        outlettype=["signal", "signal"], patcher=gen_patcher),
    newobj("plugout", "plugout~", [20, 330, 60, 22], 2, 0, []),
]

lines = [
    line("dial", 0, "scale", 0), line("scale", 0, "pSmile", 0), line("pSmile", 0, "gen", 0),
    line("tab", 0, "pStrength", 0), line("pStrength", 0, "gen", 0),
    line("gatebtn", 0, "pGate", 0), line("pGate", 0, "gen", 0),
    line("noisebtn", 0, "pNoise", 0), line("pNoise", 0, "gen", 0),
    line("plugin", 0, "gen", 0), line("plugin", 1, "gen", 1), line("phasor", 0, "gen", 2),
    line("gen", 0, "plugout", 0), line("gen", 1, "plugout", 1),
]

patcher = {"patcher": {
    "fileversion": 1, "appversion": APPVERSION, "classnamespace": "box",
    "rect": [80, 80, 1000, 520], "openinpresentation": 1,
    "default_fontsize": 10.0, "default_fontface": 0, "default_fontname": FONT,
    "gridonopen": 1, "gridsize": [8, 8], "gridsnaponopen": 1, "objectsnaponopen": 1,
    "statusbarvisible": 2, "toolbarvisible": 1, "lefttoolbarpinned": 0, "toptoolbarpinned": 0,
    "righttoolbarpinned": 0, "bottomtoolbarpinned": 0, "toolbars_unpinned_last_save": 0,
    "tallnewobj": 0, "boxanimatetime": 200, "enablehscroll": 1, "enablevscroll": 1,
    "devicewidth": 360.0, "description": "GRIN one-knob build-up", "digest": "", "tags": "",
    "style": "", "subpatcher_template": "", "assistshowspatchername": 0,
    "boxes": boxes, "lines": lines,
    "parameters": {
        "dial": ["Smile", "Smile", 0], "tab": ["Strength", "Strength", 0],
        "gatebtn": ["Gate", "Gate", 0], "noisebtn": ["Noise", "Noise", 0],
        "parameterbanks": {"0": {"index": 0, "name": "", "parameters": ["Smile", "Strength", "Gate", "Noise", "-", "-", "-", "-"]}},
        "inherited_shortname": 1,
    },
    "dependency_cache": [], "autosave": 0,
}}

js = json.dumps(patcher, indent=1)
with open(os.path.join(HERE, 'GRIN.maxpat'), 'w') as f:
    f.write(js)

# .amxd container: 'ampf' header with device type 'aaaa' (audio effect), empty meta, then the patcher JSON
payload = js.encode('utf-8') + b'\x00'
amxd = (b'ampf' + struct.pack('<I', 4) + b'aaaa'
        + b'meta' + struct.pack('<I', 4) + b'\x00\x00\x00\x00'
        + b'ptch' + struct.pack('<I', len(payload)) + payload)
with open(os.path.join(HERE, 'GRIN.amxd'), 'wb') as f:
    f.write(amxd)

print("wrote GRIN.amxd (%d bytes), GRIN.maxpat, grin.genexpr (%d lines)" % (len(amxd), code.count('\n')))
