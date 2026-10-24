{
 "patcher": {
  "fileversion": 1,
  "appversion": {
   "major": 8,
   "minor": 6,
   "revision": 2,
   "architecture": "x64",
   "modernui": 1
  },
  "classnamespace": "box",
  "rect": [
   80,
   80,
   1000,
   520
  ],
  "openinpresentation": 1,
  "default_fontsize": 10.0,
  "default_fontface": 0,
  "default_fontname": "Ableton Sans Bold",
  "gridonopen": 1,
  "gridsize": [
   8,
   8
  ],
  "gridsnaponopen": 1,
  "objectsnaponopen": 1,
  "statusbarvisible": 2,
  "toolbarvisible": 1,
  "lefttoolbarpinned": 0,
  "toptoolbarpinned": 0,
  "righttoolbarpinned": 0,
  "bottomtoolbarpinned": 0,
  "toolbars_unpinned_last_save": 0,
  "tallnewobj": 0,
  "boxanimatetime": 200,
  "enablehscroll": 1,
  "enablevscroll": 1,
  "devicewidth": 420.0,
  "description": "GRIN one-knob build-up",
  "digest": "",
  "tags": "",
  "style": "",
  "subpatcher_template": "",
  "assistshowspatchername": 0,
  "boxes": [
   {
    "box": {
     "id": "ui",
     "maxclass": "jsui",
     "patching_rect": [
      600,
      20,
      420,
      169
     ],
     "filename": "grin_ui.js",
     "presentation": 1,
     "presentation_rect": [
      0,
      0,
      420,
      169
     ],
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ],
     "border": 0,
     "parameter_enable": 0,
     "ignoreclick": 1,
     "background": 1
    }
   },
   {
    "box": {
     "id": "route",
     "maxclass": "newobj",
     "patching_rect": [
      600,
      200,
      220,
      22
     ],
     "text": "route smile strength gateon noiseon",
     "numinlets": 1,
     "numoutlets": 5,
     "outlettype": [
      "",
      "",
      "",
      "",
      ""
     ]
    }
   },
   {
    "box": {
     "id": "toDial",
     "maxclass": "newobj",
     "patching_rect": [
      600,
      230,
      50,
      22
     ],
     "text": "* 100.",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "dial",
     "maxclass": "live.numbox",
     "patching_rect": [
      20,
      20,
      44,
      15
     ],
     "presentation": 1,
     "presentation_rect": [
      14,
      18,
      136,
      136
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      "float"
     ],
     "activebgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activebgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "focusbordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activeslidercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "slidercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "tricolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "tricolor2": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetricolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetricolor2": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Smile",
       "parameter_shortname": "Smile",
       "parameter_type": 0,
       "parameter_mmax": 100.0,
       "parameter_initial_enable": 1,
       "parameter_initial": [
        0.0
       ],
       "parameter_unitstyle": 5
      }
     },
     "parameter_enable": 1,
     "varname": "Smile"
    }
   },
   {
    "box": {
     "id": "tab",
     "maxclass": "live.tab",
     "patching_rect": [
      140,
      20,
      230,
      22
     ],
     "presentation": 1,
     "presentation_rect": [
      168,
      40,
      242,
      19
     ],
     "numinlets": 1,
     "numoutlets": 3,
     "outlettype": [
      "",
      "",
      "float"
     ],
     "num_lines_patching": 1,
     "num_lines_presentation": 1,
     "activebgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activebgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "focusbordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetextcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetextoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "blinkcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Strength",
       "parameter_shortname": "Strength",
       "parameter_type": 2,
       "parameter_mmax": 6.0,
       "parameter_initial_enable": 1,
       "parameter_initial": [
        2.0
       ],
       "parameter_enum": [
        "S1",
        "S2",
        "M1",
        "M2",
        "H1",
        "H2",
        "EXT"
       ]
      }
     },
     "parameter_enable": 1,
     "varname": "Strength"
    }
   },
   {
    "box": {
     "id": "gatebtn",
     "maxclass": "live.text",
     "patching_rect": [
      140,
      60,
      60,
      20
     ],
     "presentation": 1,
     "presentation_rect": [
      168,
      134,
      62,
      24
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      ""
     ],
     "text": "GATE",
     "texton": "GATE",
     "activebgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activebgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "focusbordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetextcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetextoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "blinkcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Gate",
       "parameter_shortname": "Gate",
       "parameter_type": 2,
       "parameter_mmax": 1.0,
       "parameter_initial_enable": 1,
       "parameter_initial": [
        1.0
       ],
       "parameter_enum": [
        "off",
        "on"
       ]
      }
     },
     "parameter_enable": 1,
     "varname": "Gate"
    }
   },
   {
    "box": {
     "id": "noisebtn",
     "maxclass": "live.text",
     "patching_rect": [
      220,
      60,
      60,
      20
     ],
     "presentation": 1,
     "presentation_rect": [
      238,
      134,
      62,
      24
     ],
     "numinlets": 1,
     "numoutlets": 2,
     "outlettype": [
      "",
      ""
     ],
     "text": "NOISE",
     "texton": "NOISE",
     "activebgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activebgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bgoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "bordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "focusbordercolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "textoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetextcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "activetextoncolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "blinkcolor": [
      0.0,
      0.0,
      0.0,
      0.0
     ],
     "saved_attribute_attributes": {
      "valueof": {
       "parameter_longname": "Noise",
       "parameter_shortname": "Noise",
       "parameter_type": 2,
       "parameter_mmax": 1.0,
       "parameter_initial_enable": 1,
       "parameter_initial": [
        1.0
       ],
       "parameter_enum": [
        "off",
        "on"
       ]
      }
     },
     "parameter_enable": 1,
     "varname": "Noise"
    }
   },
   {
    "box": {
     "id": "scale",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      140,
      50,
      22
     ],
     "text": "* 0.01",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "pSmile",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      170,
      85,
      22
     ],
     "text": "prepend smile",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "pStrength",
     "maxclass": "newobj",
     "patching_rect": [
      140,
      170,
      100,
      22
     ],
     "text": "prepend strength",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "pGate",
     "maxclass": "newobj",
     "patching_rect": [
      260,
      170,
      90,
      22
     ],
     "text": "prepend gateon",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "pNoise",
     "maxclass": "newobj",
     "patching_rect": [
      370,
      170,
      95,
      22
     ],
     "text": "prepend noiseon",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   },
   {
    "box": {
     "id": "plugin",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      230,
      60,
      22
     ],
     "text": "plugin~",
     "numinlets": 2,
     "numoutlets": 2,
     "outlettype": [
      "signal",
      "signal"
     ]
    }
   },
   {
    "box": {
     "id": "phasor",
     "maxclass": "newobj",
     "patching_rect": [
      300,
      230,
      120,
      22
     ],
     "text": "phasor~ 1n @lock 1",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "signal"
     ]
    }
   },
   {
    "box": {
     "id": "gen",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      280,
      300,
      22
     ],
     "text": "gen~",
     "numinlets": 3,
     "numoutlets": 3,
     "outlettype": [
      "signal",
      "signal",
      "signal"
     ],
     "patcher": {
      "fileversion": 1,
      "appversion": {
       "major": 8,
       "minor": 6,
       "revision": 2,
       "architecture": "x64",
       "modernui": 1
      },
      "classnamespace": "dsp.gen",
      "rect": [
       100,
       100,
       900,
       700
      ],
      "gridsize": [
       15,
       15
      ],
      "boxes": [
       {
        "box": {
         "id": "g-in1",
         "maxclass": "newobj",
         "patching_rect": [
          20,
          20,
          30,
          22
         ],
         "text": "in 1",
         "numinlets": 0,
         "numoutlets": 1,
         "outlettype": [
          ""
         ]
        }
       },
       {
        "box": {
         "id": "g-in2",
         "maxclass": "newobj",
         "patching_rect": [
          200,
          20,
          30,
          22
         ],
         "text": "in 2",
         "numinlets": 0,
         "numoutlets": 1,
         "outlettype": [
          ""
         ]
        }
       },
       {
        "box": {
         "id": "g-in3",
         "maxclass": "newobj",
         "patching_rect": [
          380,
          20,
          30,
          22
         ],
         "text": "in 3",
         "numinlets": 0,
         "numoutlets": 1,
         "outlettype": [
          ""
         ]
        }
       },
       {
        "box": {
         "id": "g-code",
         "maxclass": "codebox",
         "patching_rect": [
          20,
          60,
          800,
          560
         ],
         "code": "// ---------- helpers (each call site gets its own state) ----------\ng_sstep(e0, e1, x) {\n\tt = clamp((x - e0) / (e1 - e0), 0, 1);\n\treturn t * t * (3 - 2 * t);\n}\n\ng_pick(m, a0, a1, a2, a3, a4, a5, a6) {\n\treturn m < 0.5 ? a0 : (m < 1.5 ? a1 : (m < 2.5 ? a2 : (m < 3.5 ? a3 : (m < 4.5 ? a4 : (m < 5.5 ? a5 : a6)))));\n}\n\ng_slew(x, c) {\n\tHistory y(0);\n\tHistory started(0);\n\tv = started > 0.5 ? y + c * (x - y) : x;\n\tstarted = 1;\n\ty = v;\n\treturn v;\n}\n\n// TPT state-variable filter, highpass output\ng_hp(x, fc, q, sr) {\n\tHistory ic1(0);\n\tHistory ic2(0);\n\tg = tan(3.14159265 * fc / sr);\n\tk = 1 / q;\n\ta1 = 1 / (1 + g * (g + k));\n\ta2 = g * a1;\n\ta3 = g * a2;\n\tv3 = x - ic2;\n\tv1 = a1 * ic1 + a2 * v3;\n\tv2 = ic2 + a2 * ic1 + a3 * v3;\n\tic1 = 2 * v1 - ic1;\n\tic2 = 2 * v2 - ic2;\n\treturn x - k * v1 - v2;\n}\n\n// TPT state-variable filter, bandpass output\ng_bp(x, fc, q, sr) {\n\tHistory ic1(0);\n\tHistory ic2(0);\n\tg = tan(3.14159265 * fc / sr);\n\tk = 1 / q;\n\ta1 = 1 / (1 + g * (g + k));\n\ta2 = g * a1;\n\ta3 = g * a2;\n\tv3 = x - ic2;\n\tv1 = a1 * ic1 + a2 * v3;\n\tv2 = ic2 + a2 * ic1 + a3 * v3;\n\tic1 = 2 * v1 - ic1;\n\tic2 = 2 * v2 - ic2;\n\treturn v1;\n}\n\ng_flange(x, lfo, depthms, fb, amt, sr) {\n\tDelay d(8192);\n\ttap = d.read((0.6 + depthms * lfo) * 0.001 * sr);\n\td.write(x + fb * tap);\n\treturn x + amt * (0.5 * (x + tap) - x);\n}\n\ng_pshift(x, ph, win, amt) {\n\tDelay d(16384);\n\tp2 = ph + 0.5;\n\tp2 = p2 - floor(p2);\n\tw1 = sin(3.14159265 * ph);\n\tw2 = sin(3.14159265 * p2);\n\ty = d.read(1 + (1 - ph) * win) * w1 * w1 + d.read(1 + (1 - p2) * win) * w2 * w2;\n\td.write(x);\n\treturn x + amt * (y - x);\n}\n\n// tuned feedback comb with a lowpass in the loop (tonal riser)\ng_comb(x, dsamps, fb, damp) {\n\tDelay d(8192);\n\tHistory lp(0);\n\to = d.read(dsamps);\n\tnlp = lp + damp * (o - lp);\n\tlp = nlp;\n\td.write(x + fb * nlp);\n\treturn o;\n}\n\n// freeverb-style lowpass comb and allpass\ng_fvcomb(x, len, fb, damp) {\n\tDelay d(8192);\n\tHistory f(0);\n\to = d.read(len);\n\tnf = o * (1 - damp) + f * damp;\n\tf = nf;\n\td.write(x + nf * fb);\n\treturn o;\n}\n\ng_allp(x, len) {\n\tDelay d(4096);\n\tb = d.read(len);\n\td.write(x + b * 0.5);\n\treturn b - x;\n}\n\ng_saw(freq, sr) {\n\tHistory ph(0);\n\tdt = freq / sr;\n\tp = ph + dt;\n\tp = p - floor(p);\n\tph = p;\n\ts = 2 * p - 1;\n\tt = 0;\n\tif (p < dt) {\n\t\tt = p / dt;\n\t\ts = s - (t + t - t * t - 1);\n\t}\n\telse if (p > 1 - dt) {\n\t\tt = (p - 1) / dt;\n\t\ts = s - (t * t + t + t + 1);\n\t}\n\treturn s;\n}\n\n// ---------- GRIN main ----------\n// in1/in2 = audio, in3 = phasor~ 1n @lock 1 (one ramp per bar from Live's transport)\nParam smile(0, min=0, max=1);\nParam strength(2, min=0, max=6);\nParam gateon(1, min=0, max=1);\nParam noiseon(1, min=0, max=1);\nHistory sm(0);\nHistory env(0);\nHistory lfoph(0);\nHistory pph(0);\nHistory gateenv(1);\n\nsr = samplerate;\n\n// smoothed knob\ncs = 1 - exp(-1 / (0.02 * sr));\ns = sm + cs * (smile - sm);\nsm = s;\n\n// strength mode table, glided so switching never clicks\nm = floor(strength + 0.5);\ngl = 0.002;\nmDrive  = g_slew(g_pick(m, 0.30, 0.45, 0.65, 0.80, 0.95, 1.10, 1.40), gl);\nmTop    = g_slew(g_pick(m, 3000, 3600, 4500, 5500, 6500, 7500, 9000), gl);\nmRes    = g_slew(g_pick(m, 0.5, 0.6, 0.8, 1.0, 1.1, 1.2, 1.3), gl);\nmFlange = g_slew(g_pick(m, 0.35, 0.5, 0.7, 0.85, 1.0, 1.1, 1.2), gl);\nmNoise  = g_slew(g_pick(m, 0.55, 0.7, 0.85, 1.0, 1.05, 1.1, 1.2), gl);\nmGate   = g_slew(g_pick(m, 0, 0.5, 0.7, 0.85, 0.95, 1, 1), gl);\nmPitch  = g_slew(g_pick(m, 0, 0, 0.5, 0.8, 1, 1, 1.15), gl);\nmSpace  = g_slew(g_pick(m, 0.75, 0.85, 0.9, 1, 1, 1.05, 1.1), gl);\nmTone   = g_slew(g_pick(m, 0, 0, 0, 0, 0.6, 0.85, 1), gl);\nmSynth  = g_slew(g_pick(m, 0, 0, 0, 0, 0, 0, 1), gl);\n\n// knob -> stage curves (same as GrinEngine::updateCoefficients)\ndriveMix  = clamp(s * 4, 0, 1);\ndriveAmt  = 1 + 7 * mDrive * s * s;\ndriveNorm = 1 / sqrt(driveAmt);\n\nhpMix = clamp(s * 8, 0, 1);\nhpCut = min(sr * 0.45, 20 * pow(mTop / 20, pow(s, 1.5)));\nhpQ2  = 1.3 + 1.4 * mRes * s * s;\n\nflMix   = min(0.8, 0.8 * mFlange * g_sstep(0.15, 0.75, s));\nflRate  = 0.12 + 2.8 * s * s;\nflDepth = 0.8 + 3.2 * s;\nflFb    = min(0.85, 0.75 * s * mFlange);\n\nnT     = g_sstep(0.1, 1, s);\nnLevel = noiseon > 0.5 ? 0.24 * mNoise * pow(nT, 1.5) : 0;\nnCut   = min(sr * 0.45, 300 * pow(40, s));\nnQ     = 1.1 + 1.5 * s;\n\ntT         = g_sstep(0.2, 1, s);\ntoneLevel  = mTone * pow(tT, 1.2);\nsynthLevel = mSynth * pow(g_sstep(0.35, 1, s), 1.5);\ntoneFreq   = 130.81 * pow(2, 3 * tT);\ntoneFb     = 0.92 + 0.06 * tT;\ntoneNorm   = 3.2 * (1 - toneFb) / (1 - toneFb * 0.9);\n\ngDepth = 0;\ngDiv = 1;\ngfrac = 0;\nif (gateon > 0.5 && mGate > 0.01 && s >= 0.35) {\n\tgfrac = (s - 0.35) / 0.65;\n\tgDepth = clamp((s - 0.35) / 0.05, 0, 1) * (0.3 + 0.55 * gfrac) * mGate;\n\tgDiv = s < 0.6 ? 1 : (s < 0.8 ? 0.5 : (s < 0.93 ? 0.25 : 0.125));\n}\n\npT    = g_sstep(0.55, 1, s);\npMix  = 0.35 * pT * mPitch;\npWin  = 0.045 * sr;\npStep = (pow(2, pT) - 1) / pWin;\n\nrevWet = 0.22 * mSpace * pow(s, 1.3);\ndryG   = 1 - 0.3 * s;\nroom   = min(0.98, 0.45 + 0.53 * s * mSpace);\nfvFb   = room * 0.28 + 0.7;\nfvDamp = (0.55 - 0.4 * s) * 0.4;\n\nside    = 1 + 0.7 * s;\noutG    = pow(10, 9 * pow(s, 1.5) / 20);\nclipMix = clamp(s * 5, 0, 1);\n\n// ---------- per-sample modulators ----------\ninL = in1;\ninR = in2;\nlvl = 0.5 * (abs(inL) + abs(inR));\nec = lvl > env ? exp(-1 / (0.005 * sr)) : exp(-1 / (0.3 * sr));\nenvNow = lvl + ec * (env - lvl);\nenv = envNow;\nfollow = min(1, envNow * 4);\n\nlp = lfoph + flRate / sr;\nlp = lp - floor(lp);\nlfoph = lp;\nlfoL = 0.5 + 0.5 * sin(6.2831853 * lp);\nlpR = lp + 0.25;\nlpR = lpR - floor(lpR);\nlfoR = 0.5 + 0.5 * sin(6.2831853 * lpR);\n\npp = pph + pStep;\npp = pp - floor(pp);\npph = pp;\n\ngph = in3 * 4 / gDiv;\ngph = gph - floor(gph);\ngTarget = gDepth > 0 ? (gph < 0.55 ? 1 : 0) : 1;\ngc = exp(-1 / (0.0015 * sr));\nge = gTarget + gc * (gateenv - gTarget);\ngateenv = ge;\ngGain = 1 - gDepth * (1 - ge);\n\n// synth saw riser (Extreme)\nsw1 = g_saw(toneFreq * 0.993, sr);\nsw2 = g_saw(toneFreq, sr);\nsw3 = g_saw(toneFreq * 1.007, sr);\nsynL = (sw1 + sw2 + sw3 * 0.6) * 0.12 * synthLevel * follow;\nsynR = (sw1 * 0.6 + sw2 + sw3) * 0.12 * synthLevel * follow;\n\n// ---------- channel L ----------\nxL = in1;\nxL = xL + driveMix * (tanh(xL * driveAmt) * driveNorm - xL);\nhL = g_hp(g_hp(xL, hpCut, 0.54, sr), hpCut, hpQ2, sr);\nxL = xL + hpMix * (hL - xL);\nxL = g_flange(xL, lfoL, flDepth, flFb, flMix, sr);\nxL = xL + g_bp(noise(), nCut, nQ, sr) * nLevel * follow;\nxL = g_pshift(xL, pp, pWin, pMix);\nexL = xL + 0.03 * noise() * follow;\nringL = g_comb(exL, sr / (toneFreq * 1), toneFb, 0.55)\n       + g_comb(exL, sr / (toneFreq * 1.5 * 1), toneFb, 0.55) * 0.7\n       + g_comb(exL, sr / (toneFreq * 2 * 1), toneFb, 0.55) * 0.5;\nxL = xL + ringL * toneNorm * toneLevel * follow + synL;\nxL = xL * gGain;\n\n// ---------- channel R ----------\nxR = in2;\nxR = xR + driveMix * (tanh(xR * driveAmt) * driveNorm - xR);\nhR = g_hp(g_hp(xR, hpCut, 0.54, sr), hpCut, hpQ2, sr);\nxR = xR + hpMix * (hR - xR);\nxR = g_flange(xR, lfoR, flDepth, flFb, flMix, sr);\nxR = xR + g_bp(noise(), nCut, nQ, sr) * nLevel * follow;\nxR = g_pshift(xR, pp, pWin, pMix);\nexR = xR + 0.03 * noise() * follow;\nringR = g_comb(exR, sr / (toneFreq * 1.003), toneFb, 0.55)\n       + g_comb(exR, sr / (toneFreq * 1.5 * 1.003), toneFb, 0.55) * 0.7\n       + g_comb(exR, sr / (toneFreq * 2 * 1.003), toneFb, 0.55) * 0.5;\nxR = xR + ringR * toneNorm * toneLevel * follow + synR;\nxR = xR * gGain;\n\n// ---------- reverb (4 combs + 2 allpasses per side) ----------\nk = sr / 44100;\nrin = (xL + xR) * 0.015;\nrL = g_fvcomb(rin, 1116 * k, fvFb, fvDamp) + g_fvcomb(rin, 1188 * k, fvFb, fvDamp)\n   + g_fvcomb(rin, 1277 * k, fvFb, fvDamp) + g_fvcomb(rin, 1356 * k, fvFb, fvDamp);\nrL = g_allp(g_allp(rL, 556 * k), 441 * k);\nrR = g_fvcomb(rin, 1139 * k, fvFb, fvDamp) + g_fvcomb(rin, 1211 * k, fvFb, fvDamp)\n   + g_fvcomb(rin, 1300 * k, fvFb, fvDamp) + g_fvcomb(rin, 1379 * k, fvFb, fvDamp);\nrR = g_allp(g_allp(rR, 579 * k), 464 * k);\n\noL = xL * dryG + rL * revWet * 6;\noR = xR * dryG + rR * revWet * 6;\n\n// ---------- width, makeup, safety clip ----------\nmid = 0.5 * (oL + oR);\nsd = 0.5 * (oL - oR) * side;\noL = (mid + sd) * outG;\noR = (mid - sd) * outG;\noL = oL + clipMix * (tanh(oL) - oL);\noR = oR + clipMix * (tanh(oR) - oR);\n\nout1 = oL;\nout2 = oR;\nout3 = gGain;\n",
         "fontface": 0,
         "fontname": "<Monospaced>",
         "fontsize": 12.0,
         "numinlets": 3,
         "numoutlets": 3,
         "outlettype": [
          "",
          "",
          ""
         ]
        }
       },
       {
        "box": {
         "id": "g-out1",
         "maxclass": "newobj",
         "patching_rect": [
          20,
          640,
          37,
          22
         ],
         "text": "out 1",
         "numinlets": 1,
         "numoutlets": 0,
         "outlettype": []
        }
       },
       {
        "box": {
         "id": "g-out2",
         "maxclass": "newobj",
         "patching_rect": [
          400,
          640,
          37,
          22
         ],
         "text": "out 2",
         "numinlets": 1,
         "numoutlets": 0,
         "outlettype": []
        }
       },
       {
        "box": {
         "id": "g-out3",
         "maxclass": "newobj",
         "patching_rect": [
          780,
          640,
          37,
          22
         ],
         "text": "out 3",
         "numinlets": 1,
         "numoutlets": 0,
         "outlettype": []
        }
       }
      ],
      "lines": [
       {
        "patchline": {
         "source": [
          "g-in1",
          0
         ],
         "destination": [
          "g-code",
          0
         ]
        }
       },
       {
        "patchline": {
         "source": [
          "g-in2",
          0
         ],
         "destination": [
          "g-code",
          1
         ]
        }
       },
       {
        "patchline": {
         "source": [
          "g-in3",
          0
         ],
         "destination": [
          "g-code",
          2
         ]
        }
       },
       {
        "patchline": {
         "source": [
          "g-code",
          0
         ],
         "destination": [
          "g-out1",
          0
         ]
        }
       },
       {
        "patchline": {
         "source": [
          "g-code",
          1
         ],
         "destination": [
          "g-out2",
          0
         ]
        }
       },
       {
        "patchline": {
         "source": [
          "g-code",
          2
         ],
         "destination": [
          "g-out3",
          0
         ]
        }
       }
      ]
     }
    }
   },
   {
    "box": {
     "id": "plugout",
     "maxclass": "newobj",
     "patching_rect": [
      20,
      330,
      60,
      22
     ],
     "text": "plugout~",
     "numinlets": 2,
     "numoutlets": 0,
     "outlettype": []
    }
   },
   {
    "box": {
     "id": "snap",
     "maxclass": "newobj",
     "patching_rect": [
      300,
      330,
      80,
      22
     ],
     "text": "snapshot~ 30",
     "numinlets": 2,
     "numoutlets": 1,
     "outlettype": [
      "float"
     ]
    }
   },
   {
    "box": {
     "id": "pPulse",
     "maxclass": "newobj",
     "patching_rect": [
      300,
      360,
      80,
      22
     ],
     "text": "prepend gate",
     "numinlets": 1,
     "numoutlets": 1,
     "outlettype": [
      ""
     ]
    }
   }
  ],
  "lines": [
   {
    "patchline": {
     "source": [
      "dial",
      0
     ],
     "destination": [
      "scale",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "scale",
      0
     ],
     "destination": [
      "pSmile",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pSmile",
      0
     ],
     "destination": [
      "gen",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "tab",
      0
     ],
     "destination": [
      "pStrength",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pStrength",
      0
     ],
     "destination": [
      "gen",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "gatebtn",
      0
     ],
     "destination": [
      "pGate",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pGate",
      0
     ],
     "destination": [
      "gen",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "noisebtn",
      0
     ],
     "destination": [
      "pNoise",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pNoise",
      0
     ],
     "destination": [
      "gen",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "plugin",
      0
     ],
     "destination": [
      "gen",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "plugin",
      1
     ],
     "destination": [
      "gen",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "phasor",
      0
     ],
     "destination": [
      "gen",
      2
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "gen",
      0
     ],
     "destination": [
      "plugout",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "gen",
      1
     ],
     "destination": [
      "plugout",
      1
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pSmile",
      0
     ],
     "destination": [
      "ui",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pStrength",
      0
     ],
     "destination": [
      "ui",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pGate",
      0
     ],
     "destination": [
      "ui",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pNoise",
      0
     ],
     "destination": [
      "ui",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "ui",
      0
     ],
     "destination": [
      "route",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "route",
      0
     ],
     "destination": [
      "toDial",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "toDial",
      0
     ],
     "destination": [
      "dial",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "route",
      1
     ],
     "destination": [
      "tab",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "route",
      2
     ],
     "destination": [
      "gatebtn",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "route",
      3
     ],
     "destination": [
      "noisebtn",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "gen",
      2
     ],
     "destination": [
      "snap",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "snap",
      0
     ],
     "destination": [
      "pPulse",
      0
     ]
    }
   },
   {
    "patchline": {
     "source": [
      "pPulse",
      0
     ],
     "destination": [
      "ui",
      0
     ]
    }
   }
  ],
  "parameters": {
   "dial": [
    "Smile",
    "Smile",
    0
   ],
   "tab": [
    "Strength",
    "Strength",
    0
   ],
   "gatebtn": [
    "Gate",
    "Gate",
    0
   ],
   "noisebtn": [
    "Noise",
    "Noise",
    0
   ],
   "parameterbanks": {
    "0": {
     "index": 0,
     "name": "",
     "parameters": [
      "Smile",
      "Strength",
      "Gate",
      "Noise",
      "-",
      "-",
      "-",
      "-"
     ]
    }
   },
   "inherited_shortname": 1
  },
  "dependency_cache": [],
  "autosave": 0
 }
}