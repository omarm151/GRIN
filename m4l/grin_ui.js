// GRIN for Max for Live: custom interface (jsui).
// Draws the knob, strength pills, stage meters and toggles, and talks to the
// hidden live.* parameters. In:  smile <0-1>, strength <0-6>, gateon <0/1>,
// noiseon <0/1>, gate <gain>.  Out: the same messages when the user changes them.

inlets = 1;
outlets = 1;
autowatch = 1;

mgraphics.init();
mgraphics.relative_coords = 0;
mgraphics.autofill = 0;

var W = 420, H = 169;                       // design size; drawing scales to the box
var FONT = "Ableton Sans Bold";

var val = 0, mode = 2, gateOn = 1, noiseOn = 1, gatePulse = 1;
var dragging = false, dragY = 0, dragVal = 0;

var MODES = ["S1", "S2", "M1", "M2", "H1", "H2", "EXT"];
var STAGES = ["DRIVE", "FILTER", "FLANGE", "NOISE", "TONE", "GATE", "PITCH", "SPACE"];

// same mode table as the DSP: drive, flange, noise, gate, pitch, space, tone, synth
var TABLE = [
    [0.30, 0.35, 0.55, 0.0,  0.0,  0.75, 0.0,  0],
    [0.45, 0.50, 0.70, 0.5,  0.0,  0.85, 0.0,  0],
    [0.65, 0.70, 0.85, 0.7,  0.5,  0.90, 0.0,  0],
    [0.80, 0.85, 1.00, 0.85, 0.8,  1.00, 0.0,  0],
    [0.95, 1.00, 1.05, 0.95, 1.0,  1.00, 0.6,  0],
    [1.10, 1.10, 1.10, 1.0,  1.0,  1.05, 0.85, 0],
    [1.40, 1.20, 1.20, 1.0,  1.15, 1.10, 1.0,  1]
];

// layout
var KX = 82, KY = 86, KR = 64;              // knob centre / ring radius
var RX = 168, RW = 242;                     // right column
var PILL_Y = 40, PILL_H = 19;
var METER_Y = 68, METER_H = 36;
var TOG_Y = 134, TOG_W = 62, TOG_H = 24;

// ---------------------------------------------------------------- colours --
function mixc(a, b, t) { return [a[0] + (b[0] - a[0]) * t, a[1] + (b[1] - a[1]) * t, a[2] + (b[2] - a[2]) * t]; }
var C_CYAN = [0.243, 0.902, 1.0], C_PINK = [1.0, 0.243, 0.647], C_GOLD = [1.0, 0.76, 0.243];
function colourFor(t)
{
    t = Math.max(0, Math.min(1, t));
    return t < 0.5 ? mixc(C_CYAN, C_PINK, t * 2) : mixc(C_PINK, C_GOLD, (t - 0.5) * 2);
}
function rgba(c, a) { mgraphics.set_source_rgba(c[0], c[1], c[2], a === undefined ? 1 : a); }

var BG = [0.051, 0.047, 0.071], PANEL = [0.09, 0.082, 0.122], TRACK = [0.141, 0.129, 0.18];
var DIM = [0.54, 0.52, 0.6], HI = [0.95, 0.94, 0.97];

// ----------------------------------------------------------------- stages --
function sstep(e0, e1, x) { var t = Math.max(0, Math.min(1, (x - e0) / (e1 - e0))); return t * t * (3 - 2 * t); }

function stageLevels(s)
{
    var m = TABLE[mode];
    var lv = [];
    lv[0] = Math.min(1, s * 4) * Math.min(1, s * 1.5) * Math.min(1, m[0]);
    lv[1] = Math.min(1, s * 8) * Math.pow(s, 1.2);
    lv[2] = Math.min(0.8, 0.8 * m[1] * sstep(0.15, 0.75, s)) / 0.8;
    lv[3] = noiseOn ? 0.24 * m[2] * Math.pow(sstep(0.1, 1, s), 1.5) / 0.28 : 0;
    var tT = sstep(0.2, 1, s);
    lv[4] = Math.max(m[6] * Math.pow(tT, 1.2), m[7] * Math.pow(sstep(0.35, 1, s), 1.5));
    lv[5] = (gateOn && m[3] > 0.01 && s >= 0.35)
            ? Math.min(1, (s - 0.35) / 0.05) * (0.3 + 0.55 * (s - 0.35) / 0.65) * m[3] : 0;
    lv[6] = 0.35 * sstep(0.55, 1, s) * m[4] / 0.4;
    lv[7] = 0.22 * m[5] * Math.pow(s, 1.3) / 0.24;
    for (var i = 0; i < lv.length; i++) lv[i] = Math.max(0, Math.min(1, lv[i]));
    return lv;
}

function gateDivision(s)
{
    if (!gateOn || TABLE[mode][3] < 0.01 || s < 0.35) return "--";
    return s < 0.6 ? "1/4" : s < 0.8 ? "1/8" : s < 0.93 ? "1/16" : "1/32";
}

// ------------------------------------------------------------------- draw --
function textAt(str, x, y, size, c, a, align)
{
    mgraphics.select_font_face(FONT);
    mgraphics.set_font_size(size);
    var w = mgraphics.text_measure(str)[0];
    var tx = align === "c" ? x - w / 2 : align === "r" ? x - w : x;
    rgba(c, a);
    mgraphics.move_to(tx, y);
    mgraphics.show_text(str);
}

function roundRect(x, y, w, h, r)
{
    mgraphics.rectangle_rounded(x, y, w, h, r * 2, r * 2);
}

function paint()
{
    var bw = box.rect[2] - box.rect[0], bh = box.rect[3] - box.rect[1];
    mgraphics.scale(bw / W, bh / H);

    var s = val, accent = colourFor(s);

    // background
    rgba(BG);
    mgraphics.rectangle(0, 0, W, H);
    mgraphics.fill();

    // glow behind the knob, pulsing with the gate
    var pulse = 0.6 + 0.4 * gatePulse;
    var glow = mgraphics.pattern_create_radial(KX, KY, 0, KX, KY, 110);
    glow.add_color_stop_rgba(0, accent[0], accent[1], accent[2], (0.03 + 0.28 * s) * pulse);
    glow.add_color_stop_rgba(1, accent[0], accent[1], accent[2], 0);
    mgraphics.set_source(glow);
    mgraphics.rectangle(0, 0, W, H);
    mgraphics.fill();

    drawKnob(s, accent);
    drawRight(s, accent);
}

function drawKnob(s, accent)
{
    var a0 = Math.PI * 0.75, span = Math.PI * 1.5, ang = a0 + span * s;

    mgraphics.set_line_cap("round");

    // track
    mgraphics.set_line_width(9);
    rgba(TRACK);
    mgraphics.new_path();
    mgraphics.arc(KX, KY, KR, a0, a0 + span);
    mgraphics.stroke();

    if (s > 0.002)
    {
        // soft glow
        mgraphics.set_line_width(20);
        rgba(accent, 0.16);
        mgraphics.new_path();
        mgraphics.arc(KX, KY, KR, a0, ang);
        mgraphics.stroke();

        // colour-sweeping value arc
        var seg = Math.max(2, Math.floor(s * 70));
        mgraphics.set_line_width(9);
        for (var i = 0; i < seg; i++)
        {
            var t0 = i / seg * s, t1 = (i + 1) / seg * s;
            mgraphics.set_line_cap(i === 0 || i === seg - 1 ? "round" : "butt");
            rgba(colourFor(t1));
            mgraphics.new_path();
            mgraphics.arc(KX, KY, KR, a0 + span * t0, a0 + span * t1 + 0.012);
            mgraphics.stroke();
        }
        mgraphics.set_line_cap("round");
    }

    // body
    var br = KR - 17;
    var body = mgraphics.pattern_create_linear(KX, KY - br, KX, KY + br);
    body.add_color_stop_rgba(0, 0.18, 0.165, 0.23, 1);
    body.add_color_stop_rgba(1, 0.075, 0.067, 0.098, 1);
    mgraphics.set_source(body);
    mgraphics.new_path();
    mgraphics.arc(KX, KY, br, 0, Math.PI * 2);
    mgraphics.fill();

    mgraphics.set_line_width(1);
    mgraphics.set_source_rgba(1, 1, 1, 0.07);
    mgraphics.new_path();
    mgraphics.arc(KX, KY, br, 0, Math.PI * 2);
    mgraphics.stroke();

    // the grin: flat line bending into a wide smile
    var mw = br * (0.48 + 0.14 * s), my = KY - br * 0.02, curve = br * 0.62 * s;
    var mc = s < 0.002 ? DIM : accent;
    function mouth()
    {
        mgraphics.new_path();
        mgraphics.move_to(KX - mw, my - curve * 0.25);
        mgraphics.curve_to(KX - mw * 0.45, my + curve * 0.85, KX + mw * 0.45, my + curve * 0.85, KX + mw, my - curve * 0.25);
    }
    mgraphics.set_line_width(11);
    rgba(mc, 0.22);
    mouth();
    mgraphics.stroke();
    mgraphics.set_line_width(4.5);
    rgba(mc);
    mouth();
    mgraphics.stroke();

    // pointer dot
    var px = KX + Math.cos(ang) * (br - 9), py = KY + Math.sin(ang) * (br - 9);
    rgba(mc);
    mgraphics.new_path();
    mgraphics.arc(px, py, 3.5, 0, Math.PI * 2);
    mgraphics.fill();

    // percentage in the ring's gap
    textAt(Math.round(s * 100) + "%", KX, KY + KR + 4, 13, s < 0.002 ? DIM : accent, 1, "c");
}

function drawRight(s, accent)
{
    // header
    textAt("GRIN", RX, 30, 22, HI, 1, "l");
    textAt("ONE-KNOB BUILD-UP", RX + 64, 28, 8, DIM, 1, "l");

    var div = gateDivision(s);
    textAt(div === "--" ? "" : div, RX + RW, 29, 11, accent, 0.5 + 0.5 * gatePulse, "r");

    // strength pills
    var gap = 4, pw = (RW - gap * 6) / 7;
    for (var i = 0; i < 7; i++)
    {
        var x = RX + i * (pw + gap), on = i === mode;
        var c = colourFor(i / 6);
        roundRect(x, PILL_Y, pw, PILL_H, 6);
        if (on) rgba(c, 0.22); else rgba(PANEL);
        mgraphics.fill();
        mgraphics.set_line_width(on ? 1.4 : 1);
        roundRect(x + 0.5, PILL_Y + 0.5, pw - 1, PILL_H - 1, 6);
        if (on) rgba(c); else rgba(TRACK, 1);
        mgraphics.stroke();
        textAt(MODES[i], x + pw / 2, PILL_Y + 13, 9, on ? HI : DIM, 1, "c");
    }

    // stage meters
    roundRect(RX, METER_Y, RW, METER_H + 22, 8);
    rgba(PANEL);
    mgraphics.fill();

    var lv = stageLevels(s), colW = RW / 8;
    for (var k = 0; k < 8; k++)
    {
        var cx = RX + colW * (k + 0.5), top = METER_Y + 8, bh = METER_H - 4;
        roundRect(cx - 3, top, 6, bh, 3);
        rgba(TRACK);
        mgraphics.fill();

        var v = lv[k];
        if (v > 0.01)
        {
            var sc = colourFor(k / 7), fh = Math.max(6, bh * v);
            roundRect(cx - 6, top + bh - fh - 2, 12, fh + 4, 6);
            rgba(sc, 0.22);
            mgraphics.fill();
            roundRect(cx - 3, top + bh - fh, 6, fh, 3);
            rgba(sc);
            mgraphics.fill();
        }
        textAt(STAGES[k], cx, METER_Y + METER_H + 15, 6.5, v > 0.05 ? HI : DIM, 1, "c");
    }

    // toggles
    drawToggle(RX, "GATE", gateOn);
    drawToggle(RX + TOG_W + 8, "NOISE", noiseOn);

}

function drawToggle(x, label, on)
{
    roundRect(x, TOG_Y, TOG_W, TOG_H, TOG_H / 2);
    if (on) rgba(C_PINK, 0.2); else rgba(PANEL);
    mgraphics.fill();
    mgraphics.set_line_width(1.4);
    roundRect(x + 0.5, TOG_Y + 0.5, TOG_W - 1, TOG_H - 1, TOG_H / 2);
    if (on) rgba(C_PINK); else rgba(TRACK);
    mgraphics.stroke();
    textAt(label, x + TOG_W / 2, TOG_Y + 16, 10, on ? HI : DIM, 1, "c");
}

// ---------------------------------------------------------- messages in --
function smile(v)    { val = Math.max(0, Math.min(1, v)); mgraphics.redraw(); }
function strength(v) { mode = Math.max(0, Math.min(6, Math.round(v))); mgraphics.redraw(); }
function gateon(v)   { gateOn = v > 0.5 ? 1 : 0; mgraphics.redraw(); }
function noiseon(v)  { noiseOn = v > 0.5 ? 1 : 0; mgraphics.redraw(); }
function gate(v)
{
    var g = Math.max(0, Math.min(1, v));
    if (Math.abs(g - gatePulse) > 0.02) { gatePulse = g; mgraphics.redraw(); }
}

// ------------------------------------------------------------------ mouse --
function toDesign(x, y)
{
    var bw = box.rect[2] - box.rect[0], bh = box.rect[3] - box.rect[1];
    return [x * W / bw, y * H / bh];
}

function inKnob(p) { var dx = p[0] - KX, dy = p[1] - KY; return dx * dx + dy * dy <= (KR + 12) * (KR + 12); }

function onclick(x, y, but, cmd, shift)
{
    var p = toDesign(x, y);

    if (inKnob(p))
    {
        dragging = true;
        dragY = p[1];
        dragVal = val;
        return;
    }

    var gap = 4, pw = (RW - gap * 6) / 7;
    if (p[1] >= PILL_Y && p[1] <= PILL_Y + PILL_H && p[0] >= RX && p[0] <= RX + RW)
    {
        var i = Math.min(6, Math.floor((p[0] - RX) / (pw + gap)));
        mode = i;
        outlet(0, "strength", i);
        mgraphics.redraw();
        return;
    }

    if (p[1] >= TOG_Y && p[1] <= TOG_Y + TOG_H)
    {
        if (p[0] >= RX && p[0] <= RX + TOG_W)
        {
            gateOn = gateOn ? 0 : 1;
            outlet(0, "gateon", gateOn);
        }
        else if (p[0] >= RX + TOG_W + 8 && p[0] <= RX + 2 * TOG_W + 8)
        {
            noiseOn = noiseOn ? 0 : 1;
            outlet(0, "noiseon", noiseOn);
        }
        mgraphics.redraw();
    }
}
onclick.local = 1;

function ondrag(x, y, but, cmd, shift)
{
    if (!dragging) return;
    if (!but) { dragging = false; return; }
    var p = toDesign(x, y);
    var range = shift ? 700 : 180;            // hold shift for fine control
    var v = Math.max(0, Math.min(1, dragVal + (dragY - p[1]) / range));
    if (v !== val)
    {
        val = v;
        outlet(0, "smile", v);
        mgraphics.redraw();
    }
}
ondrag.local = 1;

function ondblclick(x, y)
{
    if (inKnob(toDesign(x, y)))
    {
        val = 0;
        outlet(0, "smile", 0);
        mgraphics.redraw();
    }
}
ondblclick.local = 1;
