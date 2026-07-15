  // #figure(
  //   image("Imagenes/modeloMAT.png", width: 100%),
  //   caption: [Modelo de la máquina asincrónica trifásica],
  //   supplement: "Figura",
  // )
  
  // #equation($s_m = R_2/sqrt(R_"th"^2 + (X_"th" + X_2)^2), "(forma típica)"$)

  // #align(center)[
  //   #figure(
  //     table(
  //       columns: (auto, auto, auto, auto, auto, auto, auto),
  //       align: (center, center, center, center, center, center, center, center),

  //       [*Fracción $T_u$*], [*$P_u$[kW]*], [*$P_e$ [kW]*], [*$#math.eta$ [%]*], [*$f_p$*], [*_s_ [%]*], [*I [A]*],

  //       [0], [0.085], [0.51], [16.7], [0.056], [1,93], [2.4],
  //       [1/4], [0.255], [0.86], [29.7], [0.089], [2,67], [2.8],
  //       [2/4], [0.525], [1.5], [35.0], [0.123], [4,27], [3.2],
  //       [3/4], [0.796], [2.06], [38.6], [0.143], [5,8], [3.8],
  //       [4/4], [1.107], [2.76], [40.1], [0.151], [7,8], [4.8],
  //     ),
  //     caption: [Parámetros de funcionamiento],
  //     placement: top,
  //     supplement: "Tabla",
  //   )
  // ]

#let numEquations = counter("mycounter");
#context numEquations.step()

#let cmd(t) = text[
  #set text(font: "Verdana", fill: rgb("#4171ba"))
  _#raw(t, lang: "bash")_
]

#let lk(href, nombre) = text[
  #text(blue)[#link(href)[_#text(nombre)_]]
]

#let equation(equation) = {
  v(1em)
  block(width: 100%, inset: 0pt, {
    align(center)[
      $#equation$
    ]
    place(right, dx: -1em)[
      (#context numEquations.get().first())
    ]
  })
  context numEquations.step()
  v(1em)
}

#let project(title: "", sub: "", authors: (), date: none, body) = {
  // Set document metadata
  set document(author: authors.join(", "), title: title)
  set text(lang: "es")
  // IEEE page setup for US Letter (8.5in × 11in)
  set page(
    paper: "us-letter",
    margin: (top: 19mm, bottom: 25.4mm, left: 15.875mm, right: 15.875mm),
    columns: 2, // Enable two-column layout
    numbering: "1",
    number-align: center,
  )
  // Set text properties (IEEE uses 10pt for body text)
  set text(font: "Times New Roman", size: 10pt, lang: "es")
  // Configure headings (IEEE style: numbered, bold)
  set heading(numbering: "1.")
  show heading: it => [
    #set text(weight: "bold", size: 11pt)
    #it
    #v(0.5em)
  ]
  // Configure figures for IEEE style (9pt caption, centered images)
  show figure: it => [
    #set text(size: 9pt)
    #v(0.5em)
    #align(center)[
      #it.body
      #v(0.25em)
      #it.caption
    ]
    #v(0.5em)
  ]
  // Title page (single-column for title)
  set page(columns: 1) // Temporarily switch to single-column for title
  align(center)[
    #v(10em)
    #text(16pt, weight: "bold")[#title]
    #v(1em)
    #text(14pt, style: "italic")[#sub]
    #v(1em)
    // Render list of authors
    // #text(12pt)[#authors.join(", ")]
    #text(11pt)[#date]
    #v(1.5em)
    // Uniform image size (e.g., 80% of column width)
    #for author in authors {
      text(11pt, style: "italic")[#author]
      v(.1em)
    }

    #v(4em)
    #image("unlp_logo.png", width: 60%)

  ]
  set footnote.entry(clearance: 8em)
  // set footnote.entry(breakeable: true)
  // Switch back to two-column layout for the body
  set page(columns: 2)
  body
}

#project(
  title: "Trabajo Especial N°2",
  sub: "Sistemas Embebidos",
  authors: (
    "Tomás Vidal (69854/4)",
    "Ignacio Nahuel Chantiri (69869/1)",
  ),
  date: "9 de Julio de 2026",
)[

  = Muestreo

  La idea es tomar 256 muestras con el ADC y hacer diezmado (en 16), para luego obtener una mayor resolución, ya que los ADC del STM32F103C8T6 son de 12 bits, pero al hacer sobremuestreo, se puede alcanzar una precisión equivalente de 16 bits (#lk("https://controllerstech.com/stm32-adc-9-adc-oversampling/", "https://controllerstech.com/stm32-adc-9-adc-oversampling/")), la relación de sobremuestras a bits de precisión extra es: \
  #equation($"Bits extras" = (1/2) log_2(N)$)
  Con N siendo el número de muestras acumuladas (sobremuestras). \
  Por lo que si se sobremuestrea x256 se ganan 4 bits más de precisión. \
  _Esto se da bajo ciertas condiciones que se pueden leer en el #lk("link", "https://controllerstech.com/stm32-adc-9-adc-oversampling/")_
  la misma lo imprima en pantalla.

]
