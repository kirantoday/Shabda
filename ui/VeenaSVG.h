#pragma once

// Embedded SVG illustration of the Veena instrument.
// Diagonal playing position: yali (upper-left) to kudam (lower-right).
// Style: elegant vector art, gold and warm wood tones on transparent bg.
//
// Elements: yali (dragon head), upper gourd, tapered neck with 24 brass
// frets, large kudam (gourd body), bridge, 4 main strings, 3 thalam strings.
//
// The SVG is the STATIC base layer. Interactive overlays (string glow,
// finger position, pluck flash) are drawn on top by VeenaVisualization.

namespace veenaSVG {

// Coordinate reference for overlays:
// The SVG viewBox is 900x340.
// Yali tip: approximately (65, 35)
// Bridge: approximately (735, 245)
// Kudam center: approximately (790, 260)
// String start: ~(120, 60) to (120, 85) (4 strings spread vertically)
// String end: ~(735, 238) to (735, 252)
// Fret region: x from ~120 to ~720, spaced evenly

constexpr float viewBoxWidth  = 900.0f;
constexpr float viewBoxHeight = 340.0f;

// String Y positions at the start (yali end) and end (bridge) for overlay alignment
constexpr float stringStartX = 120.0f;
constexpr float stringEndX   = 735.0f;
constexpr float stringStartY[4] = { 62.0f, 70.0f, 78.0f, 86.0f };
constexpr float stringEndY[4]   = { 238.0f, 242.5f, 247.0f, 251.5f };

// Fret positions along X
constexpr float fretStartX = 130.0f;
constexpr float fretEndX   = 720.0f;

inline const char* getSVG()
{
    return R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 900 340">
  <defs>
    <!-- Wood gradient for neck -->
    <linearGradient id="neckWood" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#3E2723"/>
      <stop offset="30%" stop-color="#4E342E"/>
      <stop offset="60%" stop-color="#5D4037"/>
      <stop offset="100%" stop-color="#4E342E"/>
    </linearGradient>
    <!-- Wood gradient for kudam -->
    <radialGradient id="kudamWood" cx="45%" cy="40%" r="55%">
      <stop offset="0%" stop-color="#A0662B"/>
      <stop offset="50%" stop-color="#7B4B1E"/>
      <stop offset="100%" stop-color="#4E2C0A"/>
    </radialGradient>
    <!-- Wood gradient for upper gourd -->
    <radialGradient id="gourdWood" cx="40%" cy="35%" r="55%">
      <stop offset="0%" stop-color="#8B6914"/>
      <stop offset="100%" stop-color="#5D4037"/>
    </radialGradient>
    <!-- Gold gradient for yali -->
    <linearGradient id="yaliGold" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#D4AF37"/>
      <stop offset="50%" stop-color="#C4A265"/>
      <stop offset="100%" stop-color="#8B7D4A"/>
    </linearGradient>
    <!-- Brass gradient for bridge -->
    <linearGradient id="brassGrad" x1="0%" y1="0%" x2="0%" y2="100%">
      <stop offset="0%" stop-color="#D4AF37"/>
      <stop offset="100%" stop-color="#A08030"/>
    </linearGradient>
    <!-- Shadow -->
    <filter id="dropShadow">
      <feDropShadow dx="2" dy="3" stdDeviation="4" flood-color="#000000" flood-opacity="0.3"/>
    </filter>
  </defs>

  <!-- === NECK (tapered wooden beam) === -->
  <path d="M 105,48  L 730,225  L 740,255  L 115,88  Z"
        fill="url(#neckWood)" stroke="#6D4C41" stroke-width="0.8" filter="url(#dropShadow)"/>
  <!-- Neck top highlight -->
  <path d="M 107,49  L 732,226  L 730,230  L 107,53  Z"
        fill="#FFFFFF" opacity="0.06"/>
  <!-- Neck bottom shadow -->
  <path d="M 115,84  L 740,251  L 740,255  L 115,88  Z"
        fill="#000000" opacity="0.15"/>

  <!-- === FRETS (24 brass bars across neck) === -->
  <g stroke="#C4A265" opacity="0.45">
    <line x1="145" y1="53" x2="149" y2="67" stroke-width="1.2"/>
    <line x1="169" y1="57" x2="174" y2="72" stroke-width="1.2"/>
    <line x1="193" y1="62" x2="198" y2="77" stroke-width="1.2"/>
    <line x1="217" y1="67" x2="222" y2="82" stroke-width="1.2"/>
    <line x1="241" y1="72" x2="247" y2="87" stroke-width="1.2"/>
    <line x1="265" y1="77" x2="271" y2="92" stroke-width="1.2"/>
    <line x1="289" y1="82" x2="296" y2="97" stroke-width="1.2"/>
    <line x1="313" y1="87" x2="320" y2="102" stroke-width="1.2"/>
    <line x1="337" y1="92" x2="345" y2="108" stroke-width="1.2"/>
    <line x1="361" y1="97" x2="369" y2="113" stroke-width="1.2"/>
    <line x1="385" y1="102" x2="394" y2="118" stroke-width="1.2"/>
    <line x1="409" y1="107" x2="418" y2="123" stroke-width="1.2"/>
    <line x1="433" y1="112" x2="443" y2="128" stroke-width="1.2"/>
    <line x1="457" y1="117" x2="467" y2="133" stroke-width="1.2"/>
    <line x1="481" y1="122" x2="492" y2="139" stroke-width="1.2"/>
    <line x1="505" y1="127" x2="516" y2="144" stroke-width="1.2"/>
    <line x1="529" y1="132" x2="541" y2="149" stroke-width="1.2"/>
    <line x1="553" y1="138" x2="565" y2="155" stroke-width="1.2"/>
    <line x1="577" y1="143" x2="590" y2="160" stroke-width="1.2"/>
    <line x1="601" y1="148" x2="614" y2="165" stroke-width="1.2"/>
    <line x1="625" y1="153" x2="639" y2="171" stroke-width="1.2"/>
    <line x1="649" y1="158" x2="663" y2="176" stroke-width="1.2"/>
    <line x1="673" y1="163" x2="688" y2="181" stroke-width="1.2"/>
    <line x1="697" y1="168" x2="712" y2="187" stroke-width="1.2"/>
  </g>

  <!-- === KUDAM (large gourd body) === -->
  <ellipse cx="790" cy="260" rx="75" ry="62" fill="url(#kudamWood)"
           stroke="#6D4C41" stroke-width="1" filter="url(#dropShadow)"/>
  <!-- Kudam highlight -->
  <ellipse cx="775" cy="245" rx="30" ry="22" fill="#FFFFFF" opacity="0.05"/>
  <!-- Wood grain lines on kudam -->
  <g stroke="#000000" opacity="0.06" stroke-width="0.5">
    <line x1="730" y1="245" x2="850" y2="247"/>
    <line x1="735" y1="258" x2="845" y2="260"/>
    <line x1="733" y1="271" x2="847" y2="273"/>
  </g>
  <!-- Sound hole -->
  <ellipse cx="795" cy="258" rx="12" ry="10" fill="#1A1A2E" opacity="0.85"/>
  <ellipse cx="795" cy="258" rx="12" ry="10" fill="none" stroke="#C4A265" stroke-width="0.5" opacity="0.3"/>

  <!-- === BRIDGE (brass bar on kudam) === -->
  <rect x="732" y="234" width="6" height="22" rx="2" fill="url(#brassGrad)"
        stroke="#D4AF37" stroke-width="0.3" opacity="0.9"/>
  <!-- Bridge highlight -->
  <line x1="733" y1="236" x2="733" y2="254" stroke="#FFFFFF" stroke-width="0.4" opacity="0.2"/>

  <!-- === UPPER GOURD (small, near yali) === -->
  <ellipse cx="88" cy="58" rx="18" ry="22" fill="url(#gourdWood)"
           stroke="#6D4C41" stroke-width="0.7" filter="url(#dropShadow)"/>
  <!-- Gourd highlight -->
  <ellipse cx="84" cy="52" rx="7" ry="8" fill="#FFFFFF" opacity="0.05"/>

  <!-- === YALI (ornate dragon head carving) === -->
  <g filter="url(#dropShadow)">
    <!-- Head/crown -->
    <path d="M 100,42  C 85,25 60,15 50,22  C 40,28 38,38 42,42
             C 35,35 28,28 30,18  C 32,10 45,5 55,10
             C 50,4 58,0 68,5  C 75,9 78,18 75,25
             C 82,18 92,20 95,30  Z"
          fill="url(#yaliGold)" stroke="#8B7D4A" stroke-width="0.5"/>
    <!-- Jaw -->
    <path d="M 100,42  C 92,48 80,52 70,50  C 60,48 50,42 42,42
             C 48,50 58,55 68,54  C 78,53 90,50 100,42  Z"
          fill="url(#yaliGold)" stroke="#8B7D4A" stroke-width="0.5"/>
    <!-- Horn/crest detail -->
    <path d="M 55,10  C 52,2 58,-2 65,3" fill="none" stroke="#D4AF37" stroke-width="1" opacity="0.6"/>
    <!-- Eye -->
    <circle cx="62" cy="28" r="2.5" fill="#1A1A2E"/>
    <circle cx="62.5" cy="27.5" r="1" fill="#D4AF37"/>
    <!-- Nostril -->
    <circle cx="47" cy="38" r="1.2" fill="#1A1A2E" opacity="0.5"/>
    <!-- Decorative ear/scroll -->
    <path d="M 78,20  C 82,14 88,16 86,22" fill="none" stroke="#D4AF37" stroke-width="0.8" opacity="0.5"/>
  </g>

  <!-- === NECK-TO-KUDAM CONNECTOR === -->
  <path d="M 725,224  L 740,232  L 740,256  L 725,260  Z"
        fill="#4E342E" stroke="#6D4C41" stroke-width="0.5"/>

  <!-- === 4 MAIN STRINGS (yali to bridge, varying thickness) === -->
  <g>
    <!-- Sa (thickest) -->
    <line x1="105" y1="54" x2="735" y2="238" stroke="#E8D5A3" stroke-width="2.6" opacity="0.8"/>
    <!-- Pa -->
    <line x1="107" y1="62" x2="735" y2="243" stroke="#E8D5A3" stroke-width="2.0" opacity="0.75"/>
    <!-- sa -->
    <line x1="109" y1="70" x2="735" y2="248" stroke="#E8D5A3" stroke-width="1.5" opacity="0.7"/>
    <!-- Pa (upper, thinnest) -->
    <line x1="111" y1="78" x2="735" y2="252" stroke="#E8D5A3" stroke-width="1.1" opacity="0.65"/>
  </g>

  <!-- === 3 THALAM STRINGS (shorter, branching near bridge) === -->
  <g opacity="0.6">
    <line x1="710" y1="255" x2="760" y2="290" stroke="#E8D5A3" stroke-width="1.3"/>
    <line x1="700" y1="252" x2="755" y2="298" stroke="#E8D5A3" stroke-width="1.1"/>
    <line x1="690" y1="250" x2="748" y2="305" stroke="#E8D5A3" stroke-width="0.9"/>
  </g>
  <!-- Thalam labels -->
  <text x="763" y="293" fill="#D4AF37" font-size="11" font-weight="bold" font-family="sans-serif">Z</text>
  <text x="758" y="301" fill="#D4AF37" font-size="11" font-weight="bold" font-family="sans-serif">X</text>
  <text x="751" y="309" fill="#D4AF37" font-size="11" font-weight="bold" font-family="sans-serif">C</text>

  <!-- === STRING LABELS (near yali end) === -->
  <text x="82" y="55" fill="#9A8C78" font-size="7" font-family="sans-serif" text-anchor="end">Sa</text>
  <text x="82" y="63" fill="#9A8C78" font-size="7" font-family="sans-serif" text-anchor="end">Pa</text>
  <text x="82" y="71" fill="#9A8C78" font-size="7" font-family="sans-serif" text-anchor="end">sa</text>
  <text x="82" y="79" fill="#9A8C78" font-size="7" font-family="sans-serif" text-anchor="end">Pa</text>
</svg>
)SVG";
}

} // namespace veenaSVG
