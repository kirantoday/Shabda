#pragma once

// Embedded SVG illustration of the Veena — VERTICAL orientation.
// Yali at TOP, kudam at BOTTOM. Near-vertical (~10-15 deg tilt).
// Strings run top-to-bottom as the most prominent visual element.
//
// The SVG is the STATIC base layer. Interactive overlays (string glow,
// finger position, pluck flash) are drawn on top by VeenaVisualization.

namespace veenaSVG {

// viewBox: tall and narrow to match vertical veena
constexpr float viewBoxWidth  = 200.0f;
constexpr float viewBoxHeight = 600.0f;

// String coordinates for overlay alignment.
// Strings run from near yali (top) to bridge (bottom).
// 4 strings spread horizontally, spaced ~10px apart.
constexpr float stringTopY    = 95.0f;   // just below upper gourd
constexpr float stringBottomY = 480.0f;  // at the bridge
constexpr float stringX[4]    = { 85.0f, 93.0f, 101.0f, 109.0f };  // well-spaced

// Fret region
constexpr float fretTopY    = 100.0f;
constexpr float fretBottomY = 475.0f;

// Kudam center
constexpr float kudamCenterX = 100.0f;
constexpr float kudamCenterY = 530.0f;

inline const char* getSVG()
{
    return R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 200 600">
  <defs>
    <linearGradient id="neckWood" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#3E2723"/>
      <stop offset="30%" stop-color="#4E342E"/>
      <stop offset="70%" stop-color="#5D4037"/>
      <stop offset="100%" stop-color="#4E342E"/>
    </linearGradient>
    <radialGradient id="kudamWood" cx="45%" cy="40%" r="55%">
      <stop offset="0%" stop-color="#A0662B"/>
      <stop offset="50%" stop-color="#7B4B1E"/>
      <stop offset="100%" stop-color="#4E2C0A"/>
    </radialGradient>
    <radialGradient id="gourdWood" cx="40%" cy="35%" r="55%">
      <stop offset="0%" stop-color="#8B6914"/>
      <stop offset="100%" stop-color="#5D4037"/>
    </radialGradient>
    <linearGradient id="yaliGold" x1="0%" y1="0%" x2="100%" y2="100%">
      <stop offset="0%" stop-color="#D4AF37"/>
      <stop offset="50%" stop-color="#C4A265"/>
      <stop offset="100%" stop-color="#8B7D4A"/>
    </linearGradient>
    <linearGradient id="brassGrad" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#D4AF37"/>
      <stop offset="100%" stop-color="#A08030"/>
    </linearGradient>
    <filter id="shadow">
      <feDropShadow dx="1" dy="2" stdDeviation="3" flood-color="#000" flood-opacity="0.3"/>
    </filter>
  </defs>

  <!-- ============ YALI (dragon head at top) ============ -->
  <g transform="translate(97,10)" filter="url(#shadow)">
    <!-- Head crown -->
    <path d="M 0,38  C -8,20 -20,10 -25,15  C -30,20 -28,32 -22,35
             C -28,28 -35,22 -32,12  C -28,4 -18,0 -10,5
             C -15,-2 -8,-5 2,0  C 8,4 10,14 8,22
             C 14,16 20,18 18,28  Z"
          fill="url(#yaliGold)" stroke="#8B7D4A" stroke-width="0.5"/>
    <!-- Jaw -->
    <path d="M 0,38  C -5,44 -14,48 -20,46  C -26,44 -28,38 -22,35
             C -18,42 -10,46 -4,44  C 2,42 2,40 0,38  Z"
          fill="url(#yaliGold)" stroke="#8B7D4A" stroke-width="0.5"/>
    <!-- Horn -->
    <path d="M -10,5  C -12,-3 -6,-6 0,0" fill="none" stroke="#D4AF37" stroke-width="1" opacity="0.6"/>
    <!-- Eye -->
    <circle cx="-8" cy="22" r="2.2" fill="#1A1A2E"/>
    <circle cx="-7.5" cy="21.5" r="0.9" fill="#D4AF37"/>
    <!-- Ear scroll -->
    <path d="M 10,16  C 14,10 18,13 16,18" fill="none" stroke="#D4AF37" stroke-width="0.7" opacity="0.5"/>
  </g>

  <!-- ============ UPPER GOURD ============ -->
  <ellipse cx="97" cy="68" rx="22" ry="18" fill="url(#gourdWood)"
           stroke="#6D4C41" stroke-width="0.7" filter="url(#shadow)"/>
  <!-- Tuning pegs (small gold dots on sides) -->
  <circle cx="73" cy="62" r="2.5" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>
  <circle cx="73" cy="70" r="2.5" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>
  <circle cx="121" cy="62" r="2.5" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>
  <circle cx="121" cy="70" r="2.5" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>

  <!-- ============ NECK (long, running downward) ============ -->
  <rect x="78" y="85" width="38" height="400" rx="3"
        fill="url(#neckWood)" stroke="#6D4C41" stroke-width="0.6" filter="url(#shadow)"/>
  <!-- Neck highlight (left edge) -->
  <rect x="78" y="85" width="4" height="400" rx="2" fill="#FFF" opacity="0.04"/>
  <!-- Neck shadow (right edge) -->
  <rect x="112" y="85" width="4" height="400" rx="2" fill="#000" opacity="0.08"/>

  <!-- ============ FRETS (24 horizontal gold bars) ============ -->
  <g stroke="#C4A265" stroke-width="1.2" opacity="0.5">
    <line x1="79" y1="101" x2="115" y2="101"/>
    <line x1="79" y1="117" x2="115" y2="117"/>
    <line x1="79" y1="133" x2="115" y2="133"/>
    <line x1="79" y1="149" x2="115" y2="149"/>
    <line x1="79" y1="165" x2="115" y2="165"/>
    <line x1="79" y1="181" x2="115" y2="181"/>
    <line x1="79" y1="197" x2="115" y2="197"/>
    <line x1="79" y1="213" x2="115" y2="213"/>
    <line x1="79" y1="229" x2="115" y2="229"/>
    <line x1="79" y1="245" x2="115" y2="245"/>
    <line x1="79" y1="261" x2="115" y2="261"/>
    <line x1="79" y1="277" x2="115" y2="277"/>
    <line x1="79" y1="293" x2="115" y2="293"/>
    <line x1="79" y1="309" x2="115" y2="309"/>
    <line x1="79" y1="325" x2="115" y2="325"/>
    <line x1="79" y1="341" x2="115" y2="341"/>
    <line x1="79" y1="357" x2="115" y2="357"/>
    <line x1="79" y1="373" x2="115" y2="373"/>
    <line x1="79" y1="389" x2="115" y2="389"/>
    <line x1="79" y1="405" x2="115" y2="405"/>
    <line x1="79" y1="421" x2="115" y2="421"/>
    <line x1="79" y1="437" x2="115" y2="437"/>
    <line x1="79" y1="453" x2="115" y2="453"/>
    <line x1="79" y1="469" x2="115" y2="469"/>
  </g>

  <!-- ============ KUDAM (large gourd at bottom) ============ -->
  <ellipse cx="100" cy="530" rx="58" ry="48" fill="url(#kudamWood)"
           stroke="#6D4C41" stroke-width="0.8" filter="url(#shadow)"/>
  <!-- Wood grain -->
  <g stroke="#000" opacity="0.06" stroke-width="0.5">
    <line x1="55" y1="520" x2="145" y2="520"/>
    <line x1="52" y1="532" x2="148" y2="532"/>
    <line x1="55" y1="544" x2="145" y2="544"/>
  </g>
  <!-- Sound hole -->
  <ellipse cx="100" cy="530" rx="11" ry="9" fill="#1A1A2E" opacity="0.85"/>
  <ellipse cx="100" cy="530" rx="11" ry="9" fill="none" stroke="#C4A265"
           stroke-width="0.5" opacity="0.3"/>
  <!-- Kudam highlight -->
  <ellipse cx="88" cy="518" rx="18" ry="14" fill="#FFF" opacity="0.04"/>

  <!-- ============ BRIDGE (gold bar on top of kudam) ============ -->
  <rect x="81" y="479" width="32" height="5" rx="1.5" fill="url(#brassGrad)"
        stroke="#D4AF37" stroke-width="0.3" opacity="0.9"/>
  <!-- Bridge highlight -->
  <line x1="82" y1="480" x2="112" y2="480" stroke="#FFF" stroke-width="0.4" opacity="0.15"/>

  <!-- ============ 4 MAIN STRINGS (top to bottom, well-spaced) ============ -->
  <g>
    <!-- Sa (thickest, leftmost) -->
    <line x1="85" y1="95" x2="85" y2="480" stroke="#E8D5A3" stroke-width="2.8" opacity="0.85"/>
    <!-- Pa -->
    <line x1="93" y1="95" x2="93" y2="480" stroke="#E8D5A3" stroke-width="2.2" opacity="0.80"/>
    <!-- sa -->
    <line x1="101" y1="95" x2="101" y2="480" stroke="#E8D5A3" stroke-width="1.7" opacity="0.75"/>
    <!-- Pa (thinnest, rightmost) -->
    <line x1="109" y1="95" x2="109" y2="480" stroke="#E8D5A3" stroke-width="1.2" opacity="0.70"/>
  </g>

  <!-- String labels at top -->
  <text x="85" y="92" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">Sa</text>
  <text x="93" y="92" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">Pa</text>
  <text x="101" y="92" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">sa</text>
  <text x="109" y="92" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">Pa</text>

  <!-- ============ 3 THALAM STRINGS (branching near bridge) ============ -->
  <g opacity="0.65">
    <line x1="87" y1="470" x2="55" y2="520" stroke="#E8D5A3" stroke-width="1.4"/>
    <line x1="90" y1="473" x2="50" y2="535" stroke="#E8D5A3" stroke-width="1.2"/>
    <line x1="93" y1="476" x2="46" y2="548" stroke="#E8D5A3" stroke-width="1.0"/>
  </g>
  <!-- Thalam labels -->
  <text x="48" y="519" fill="#D4AF37" font-size="10" font-weight="bold" font-family="sans-serif">Z</text>
  <text x="43" y="534" fill="#D4AF37" font-size="10" font-weight="bold" font-family="sans-serif">X</text>
  <text x="39" y="548" fill="#D4AF37" font-size="10" font-weight="bold" font-family="sans-serif">C</text>
</svg>
)SVG";
}

} // namespace veenaSVG
