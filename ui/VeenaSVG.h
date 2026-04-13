#pragma once

// Embedded SVG illustration of the Veena — VERTICAL orientation.
// Yali at TOP, kudam at BOTTOM. Strings run full length.
// Proportions matched to real instrument reference photo.

namespace veenaSVG {

constexpr float viewBoxWidth  = 200.0f;
constexpr float viewBoxHeight = 700.0f;

// String coordinates for overlay alignment.
constexpr float stringTopY    = 120.0f;
constexpr float stringBottomY = 545.0f;
constexpr float stringX[4]    = { 84.0f, 92.0f, 100.0f, 108.0f };

// Fret region
constexpr float fretTopY    = 125.0f;
constexpr float fretBottomY = 540.0f;

// Kudam center
constexpr float kudamCenterX = 97.0f;
constexpr float kudamCenterY = 610.0f;

inline const char* getSVG()
{
    return R"SVG(
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 200 700">
  <defs>
    <linearGradient id="neckWood" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#3E2723"/>
      <stop offset="30%" stop-color="#4E342E"/>
      <stop offset="70%" stop-color="#5D4037"/>
      <stop offset="100%" stop-color="#4E342E"/>
    </linearGradient>
    <radialGradient id="kudamWood" cx="45%" cy="42%" r="52%">
      <stop offset="0%" stop-color="#A0662B"/>
      <stop offset="50%" stop-color="#7B4B1E"/>
      <stop offset="100%" stop-color="#4E2C0A"/>
    </radialGradient>
    <radialGradient id="gourdWood" cx="40%" cy="35%" r="55%">
      <stop offset="0%" stop-color="#8B6914"/>
      <stop offset="100%" stop-color="#5D4037"/>
    </radialGradient>
    <linearGradient id="yaliGold" x1="30%" y1="0%" x2="70%" y2="100%">
      <stop offset="0%" stop-color="#D4AF37"/>
      <stop offset="50%" stop-color="#C4A265"/>
      <stop offset="100%" stop-color="#8B7D4A"/>
    </linearGradient>
    <linearGradient id="brassGrad" x1="0%" y1="0%" x2="100%" y2="0%">
      <stop offset="0%" stop-color="#D4AF37"/>
      <stop offset="100%" stop-color="#A08030"/>
    </linearGradient>
    <filter id="shadow">
      <feDropShadow dx="1" dy="2" stdDeviation="2.5" flood-color="#000" flood-opacity="0.25"/>
    </filter>
  </defs>

  <!-- ============ YALI (elegant dragon head, tall ~2:1 ratio) ============ -->
  <g transform="translate(96,20)" filter="url(#shadow)">
    <!-- Main head — tall upward-curving carved shape -->
    <path d="M 0,55
             C -3,45 -6,35 -10,28
             C -14,20 -18,14 -20,8
             C -22,2 -18,-5 -12,-8
             C -6,-11 0,-10 4,-6
             C 8,-2 10,4 10,10
             C 12,4 16,2 18,6
             C 20,10 18,18 14,24
             C 10,30 6,40 2,50
             Z"
          fill="url(#yaliGold)" stroke="#8B7D4A" stroke-width="0.5"/>
    <!-- Jaw / lower mandible -->
    <path d="M 0,55  C -4,60 -12,62 -18,58
             C -22,54 -22,48 -18,44
             C -14,50 -8,56 0,55  Z"
          fill="url(#yaliGold)" stroke="#8B7D4A" stroke-width="0.4"/>
    <!-- Crown crest -->
    <path d="M -12,-8  C -14,-16 -8,-20 -2,-14  C 0,-18 6,-16 4,-10"
          fill="none" stroke="#D4AF37" stroke-width="0.8" opacity="0.7"/>
    <!-- Eye -->
    <circle cx="-6" cy="20" r="2.5" fill="#1A1A2E"/>
    <circle cx="-5.5" cy="19.5" r="1" fill="#D4AF37"/>
    <!-- Nostril -->
    <ellipse cx="-14" cy="40" rx="1.5" ry="1" fill="#1A1A2E" opacity="0.4"/>
    <!-- Decorative scroll near ear -->
    <path d="M 12,14  C 16,8 20,12 17,17" fill="none" stroke="#D4AF37" stroke-width="0.6" opacity="0.5"/>
    <!-- Mouth line -->
    <path d="M -18,44  C -22,42 -24,38 -20,35" fill="none" stroke="#8B7D4A" stroke-width="0.4" opacity="0.4"/>
  </g>

  <!-- ============ UPPER GOURD (small, spherical) ============ -->
  <ellipse cx="96" cy="92" rx="15" ry="14" fill="url(#gourdWood)"
           stroke="#6D4C41" stroke-width="0.6" filter="url(#shadow)"/>
  <!-- Tuning pegs -->
  <circle cx="78" cy="86" r="2.2" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>
  <circle cx="78" cy="94" r="2.2" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>
  <circle cx="114" cy="86" r="2.2" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>
  <circle cx="114" cy="94" r="2.2" fill="#C4A265" stroke="#8B7D4A" stroke-width="0.3"/>

  <!-- ============ NECK ============ -->
  <rect x="78" y="105" width="38" height="445" rx="3"
        fill="url(#neckWood)" stroke="#6D4C41" stroke-width="0.5" filter="url(#shadow)"/>
  <rect x="78" y="105" width="4" height="445" rx="2" fill="#FFF" opacity="0.035"/>
  <rect x="112" y="105" width="4" height="445" rx="2" fill="#000" opacity="0.07"/>

  <!-- ============ FRETS (24, logarithmic spacing: wider at top) ============ -->
  <g stroke="#C4A265" stroke-width="1.2" opacity="0.5">
    <line x1="79" y1="128" x2="115" y2="128"/>
    <line x1="79" y1="150" x2="115" y2="150"/>
    <line x1="79" y1="170" x2="115" y2="170"/>
    <line x1="79" y1="189" x2="115" y2="189"/>
    <line x1="79" y1="207" x2="115" y2="207"/>
    <line x1="79" y1="224" x2="115" y2="224"/>
    <line x1="79" y1="240" x2="115" y2="240"/>
    <line x1="79" y1="255" x2="115" y2="255"/>
    <line x1="79" y1="270" x2="115" y2="270"/>
    <line x1="79" y1="284" x2="115" y2="284"/>
    <line x1="79" y1="298" x2="115" y2="298"/>
    <line x1="79" y1="311" x2="115" y2="311"/>
    <line x1="79" y1="324" x2="115" y2="324"/>
    <line x1="79" y1="336" x2="115" y2="336"/>
    <line x1="79" y1="348" x2="115" y2="348"/>
    <line x1="79" y1="360" x2="115" y2="360"/>
    <line x1="79" y1="372" x2="115" y2="372"/>
    <line x1="79" y1="383" x2="115" y2="383"/>
    <line x1="79" y1="394" x2="115" y2="394"/>
    <line x1="79" y1="405" x2="115" y2="405"/>
    <line x1="79" y1="416" x2="115" y2="416"/>
    <line x1="79" y1="427" x2="115" y2="427"/>
    <line x1="79" y1="437" x2="115" y2="437"/>
    <line x1="79" y1="447" x2="115" y2="447"/>
  </g>

  <!-- ============ KUDAM (nearly circular, large) ============ -->
  <ellipse cx="97" cy="610" rx="56" ry="54" fill="url(#kudamWood)"
           stroke="#6D4C41" stroke-width="0.8" filter="url(#shadow)"/>
  <g stroke="#000" opacity="0.05" stroke-width="0.5">
    <line x1="55" y1="600" x2="139" y2="600"/>
    <line x1="52" y1="612" x2="142" y2="612"/>
    <line x1="55" y1="624" x2="139" y2="624"/>
  </g>
  <!-- Sound hole -->
  <ellipse cx="97" cy="612" rx="10" ry="10" fill="#1A1A2E" opacity="0.85"/>
  <ellipse cx="97" cy="612" rx="10" ry="10" fill="none" stroke="#C4A265"
           stroke-width="0.5" opacity="0.3"/>
  <!-- Highlight -->
  <ellipse cx="86" cy="598" rx="16" ry="14" fill="#FFF" opacity="0.035"/>

  <!-- ============ BRIDGE ============ -->
  <rect x="80" y="548" width="34" height="5" rx="1.5" fill="url(#brassGrad)"
        stroke="#D4AF37" stroke-width="0.3" opacity="0.9"/>
  <line x1="81" y1="549" x2="113" y2="549" stroke="#FFF" stroke-width="0.4" opacity="0.12"/>

  <!-- ============ 4 MAIN STRINGS ============ -->
  <g>
    <line x1="84" y1="120" x2="84" y2="549" stroke="#E8D5A3" stroke-width="2.8" opacity="0.85"/>
    <line x1="92" y1="120" x2="92" y2="549" stroke="#E8D5A3" stroke-width="2.2" opacity="0.80"/>
    <line x1="100" y1="120" x2="100" y2="549" stroke="#E8D5A3" stroke-width="1.7" opacity="0.75"/>
    <line x1="108" y1="120" x2="108" y2="549" stroke="#E8D5A3" stroke-width="1.2" opacity="0.70"/>
  </g>
  <text x="84" y="116" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">Sa</text>
  <text x="92" y="116" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">Pa</text>
  <text x="100" y="116" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">sa</text>
  <text x="108" y="116" fill="#9A8C78" font-size="6" font-family="sans-serif" text-anchor="middle">Pa</text>

  <!-- ============ 3 THALAM STRINGS ============ -->
  <g opacity="0.65">
    <line x1="86" y1="538" x2="52" y2="590" stroke="#E8D5A3" stroke-width="1.4"/>
    <line x1="88" y1="541" x2="47" y2="605" stroke="#E8D5A3" stroke-width="1.2"/>
    <line x1="90" y1="544" x2="43" y2="618" stroke="#E8D5A3" stroke-width="1.0"/>
  </g>
  <text x="45" y="589" fill="#D4AF37" font-size="10" font-weight="bold" font-family="sans-serif">Z</text>
  <text x="40" y="604" fill="#D4AF37" font-size="10" font-weight="bold" font-family="sans-serif">X</text>
  <text x="36" y="618" fill="#D4AF37" font-size="10" font-weight="bold" font-family="sans-serif">C</text>
</svg>
)SVG";
}

} // namespace veenaSVG
