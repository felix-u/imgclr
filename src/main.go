package main

import (
    "os"
    "fmt"
    _ "image/png"
    _ "image/jpeg"
)

const usage_text = `
imgclr - image colouriser 

Usage: imgclr <input file> <output file> <palette...> [options]

Options:
      --dither <algorithm>
        Specify dithering algorithm - one of:
            'floyd-steinberg' (default), 'none', 'atkinson', 'jjn', 
            'burkes', 'sierra-lite'
      --invert
        Invert the image's luminance
      --palette <hex>...
        Specify palette - at least two (2) space-separated hex colours
  -h, --help
        Print this help and exit
`

func main() {
    for _, arg := range os.Args[1:] {
        if (arg == "-h" || arg == "--help") {
            fmt.Print(usage_text)
            os.Exit(0)
        }
    }

    if len(os.Args) < 3 {
        fmt.Print(usage_text)
        os.Exit(1)
    }

    var infile_path = os.Args[1]
    // var outfile_path = os.Args[2]

    infile, err := os.ReadFile(infile_path)
    if err != nil {
        panic(err)
    }
    fmt.Print(infile)
}
