import os
import sys
from PIL import Image
from PyQt6.QtWidgets import (
    QApplication,
    QFileDialog,
    QMessageBox,
    QDialog,
    QVBoxLayout,
    QTextEdit,
    QPushButton,
    QLabel,
    QHBoxLayout,
)
from PyQt6.QtGui import QPixmap, QImage
import re

def png_to_c_bitmap(png_file, output_file):
    img = Image.open(png_file)

    # Convert to RGB if necessary
    if img.mode != 'RGB':
        img = img.convert('RGB')

    width, height = img.size
    pixels = img.load()

    # Convert RGB to 8-bit color (R3G3B2)
    bitmap_data = [width & 0xFF, height & 0xFF]  # store low byte of width/height (match original behaviour)
    for y in range(height):
        for x in range(width):
            r, g, b = pixels[x, y][:3]
            color_8bit = ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6)
            bitmap_data.append(color_8bit)

    # Ensure output directory exists
    out_dir = os.path.dirname(output_file)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    with open(output_file, 'w', newline='\n') as f:
        f.write("#ifndef BITMAP_H\n")
        f.write("#define BITMAP_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define BITMAP_WIDTH {width}\n")
        f.write(f"#define BITMAP_HEIGHT {height}\n\n")
        f.write(f"const uint8_t bitmap_data[{len(bitmap_data)}] = {{\n")

        for i, byte in enumerate(bitmap_data):
            if i % 16 == 0:
                f.write("    ")
            f.write(f"0x{byte:02X}")
            if i < len(bitmap_data) - 1:
                f.write(", ")
            if (i + 1) % 16 == 0:
                f.write("\n")

        f.write("\n};\n\n")
        f.write("#endif // BITMAP_H\n")

def load_bitmap_from_header(header_file):
    """Extract bitmap data and dimensions from .h file."""
    try:
        with open(header_file, 'r') as f:
            content = f.read()

        # Extract hex values
        hex_values = re.findall(r'0x([0-9A-Fa-f]{2})', content)
        bitmap_data = [int(h, 16) for h in hex_values]
        
        width = bitmap_data[0]
        height = bitmap_data[1]
        
        return width, height, bitmap_data
    except Exception as e:
        raise Exception(f"Failed to parse header file: {e}")

def bitmap_to_image(width, height, bitmap_data):
    """Convert 8-bit bitmap data (R3G3B2) back to PIL Image."""
    img = Image.new('RGB', (width, height))
    pixels = img.load()
    
    # Skip first 2 bytes (width and height)
    data_index = 2
    
    for y in range(height):
        for x in range(width):
            if data_index >= len(bitmap_data):
                break
            
            color_8bit = bitmap_data[data_index]
            data_index += 1
            
            # Decode R3G3B2
            r = (color_8bit >> 5) << 5
            g = ((color_8bit >> 2) & 0x07) << 5
            b = (color_8bit & 0x03) << 6
            
            pixels[x, y] = (r, g, b)
    
    return img

def show_file_contents_dialog(parent, path):
    try:
        width, height, bitmap_data = load_bitmap_from_header(path)
        img = bitmap_to_image(width, height, bitmap_data)
        
        with open(path, 'r', encoding='utf-8', errors='replace') as f:
            contents = f.read()
    except Exception as e:
        QMessageBox.critical(parent, "Error", f"Failed to read file:\n{e}")
        return

    dlg = QDialog(parent)
    dlg.setWindowTitle(f"Contents: {os.path.basename(path)}")
    dlg.resize(1600, 800)
    layout = QHBoxLayout(dlg)  # Changed to QHBoxLayout for side by side

    text = QTextEdit(dlg)
    text.setReadOnly(True)
    text.setPlainText(contents)
    layout.addWidget(text)
    
    # Scale up for visibility
    scale = max(1, 200 // max(1, max(width, height)))
    img_scaled = img.resize((width * scale, height * scale), Image.NEAREST)
    
    # Create QImage (provide bytesPerLine)
    b = img_scaled.tobytes()
    bytes_per_line = img_scaled.width * 3
    qimg = QImage(b, img_scaled.width, img_scaled.height, bytes_per_line, QImage.Format.Format_RGB888)
    label = QLabel(dlg)
    label.setPixmap(QPixmap.fromImage(qimg))
    layout.addWidget(label)

    btn_close = QPushButton("Close", dlg)
    btn_close.clicked.connect(dlg.accept)
    layout.addWidget(btn_close)

    dlg.exec()


def pick_and_convert(parent=None):
    """Select a PNG and save as C header. parent should be a QWidget (dialog) or None."""
    png_path, _ = QFileDialog.getOpenFileName(
        parent,
        "Select PNG image",
        "",
        "PNG Images (*.png *.PNG);;All Files (*)"
    )
    if not png_path:
        return

    default_name = os.path.splitext(os.path.basename(png_path))[0] + ".h"
    default_dir = os.path.dirname(png_path) or ""

    save_path, _ = QFileDialog.getSaveFileName(
        parent,
        "Save C header",
        os.path.join(default_dir, default_name),
        "C Header Files (*.h);;All Files (*)"
    )
    if not save_path:
        return

    try:
        png_to_c_bitmap(png_path, save_path)
        show_file_contents_dialog(parent, save_path)
    except Exception as e:
        QMessageBox.critical(parent, "Error", f"Failed to convert image:\n{e}")

def load_bitmap_dialog(parent=None):
    """Load and display a bitmap from .h file. parent should be a QWidget (dialog) or None."""
    header_path, _ = QFileDialog.getOpenFileName(
        parent,
        "Select .h bitmap file",
        "",
        "C Header Files (*.h);;All Files (*)"
    )
    if header_path:
        show_file_contents_dialog(parent, header_path)

def main_menu():
    app = QApplication(sys.argv)

    dlg = QDialog()
    dlg.setWindowTitle("Choose action")
    dlg.resize(300, 150)
    layout = QVBoxLayout(dlg)

    btn_convert = QPushButton("Convert PNG to .h", dlg)
    btn_load = QPushButton("Load and show .h bitmap", dlg)
    btn_exit = QPushButton("Exit", dlg)

    layout.addWidget(btn_convert)
    layout.addWidget(btn_load)
    layout.addWidget(btn_exit)

    btn_convert.clicked.connect(lambda: pick_and_convert(dlg))
    btn_load.clicked.connect(lambda: load_bitmap_dialog(dlg))
    btn_exit.clicked.connect(dlg.accept)

    dlg.exec()

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('--load', action='store_true', help='Load and display bitmap from .h file')
    args = parser.parse_args()
    
    if args.load:
        # If user called with --load, still create an app and open the load dialog directly
        app = QApplication(sys.argv)
        load_bitmap_dialog(None)
    else:
        main_menu()