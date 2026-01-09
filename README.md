# Projet Avancé - Stéganographie

## Objective
This project focuses on manipulating images in PNG format and implementing steganography techinques to hide and extract data (text or image) inside an image.

Main features:
- Loading and saving PNG files.
- Displaying the content of a PNG file in the terminal using hexadecimal blocks.
- Hiding and extracting text using the LSB technique.
- Hiding and extracting an image inside another image (using the LSB technique too).
- Graphical display of PNG files using SDL2 (bonus)

---

## Project Structure

````
.
├── main.c              # Main program (command-line handling)
├── png_utils.c/.h      # PNG loading, saving, and hexadecimal display
├── steg_text.c/.h      # Text steganography (LSB)
├── steg_image.c/.h     # Image steganography (LSB)
├── display_sdl.c/.h    # Graphical display using SDL2 (bonus)
├── Makefile            # Project compilation
├── images/             # Test images
│   ├── input.png
│   ├── secret.png
│   ├── stego_img.png
│   └── extracted.png
└── README.md           # Project documentation
````

---

## Dependencies

The project relies on the following libraries:
- **libpng** : reading and writing PNG files.
- **SDL2** : graphical display.

### Installing Dependencies

#### macOS (Homebrew)
```bash
brew install libpng sdl2 pkg-config
```

#### Ubuntu
```bash
sudo apt update
sudo apt install libpng-dev libsdl2-dev pkg-config
```

---

## Compilation

To compile the project, run the following command in the root directory:

```bash
make
```

To clean compiled files:
```bash
make clean
```

---

## Usage

The program is executed from the command line using the following commands (note that the files name are references): 

### Load and save a PNG file
```bash
./steg_png open-and-save <input.png> <output.png>
```

### Display a PNG file in hexadecimal format (terminal)
```bash
./steg_png show-hex <input.png> [nb_lignes]
```

### Hide text inside an image
```bash
./steg_png hide-text <input.png> <output.png> "message"
```

### Extract hidden text
```bash
./steg_png extract-text <stego.png>
```

### Hide an image inside another image
```bash
./steg_png hide-image <cover.png> <secret.png> <output.png>
```

### Extract a hidden image
```bash
./steg_png extract-image <stego.png> <extracted.png>
```

### Display a PNG file graphically (bonus SDL2)
```bash
./steg_png show-image <input.png>
```

---

## Remarks

- The secret image must be smaller than the cover image, as the storage capacity depends on the number of available pixels.
- When hiding an image, a color depth reduction is applied to limit the visual impact on the cover image.
- Steganography algorithms use only the least significant bits (LSB) of the RGB channels.

---

## Example – Image Steganography

Below is an example of image steganography using the LSB method.

- **Cover image:** `input.png`
- **Secret image:** `secret.png`
- **Stego image:** `stego_img.png`
- **Extracted image:** `extracted.png`

### Comparison

| Cover image | Stego image |
|------------|-------------|
| ![](images/input.png) | ![](images/stego_img.png) |

### Extracted Secret Image

![Extracted image](images/extracted.png)

---

## Author

Project developped by **Eunice Saraí CASTILLO TURRUBIARTES**.

---
