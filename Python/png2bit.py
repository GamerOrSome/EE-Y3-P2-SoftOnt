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

def png_to_c_bitmap(png_file, output_file, use_filename=False):
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

    # Generate variable name from filename
    if use_filename:
        filename_base = os.path.splitext(os.path.basename(png_file))[0]
        # Sanitize filename: replace hyphens and spaces with underscores, remove invalid characters
        filename_base = re.sub(r'[^a-zA-Z0-9_]', '_', filename_base)
        var_name = f"{filename_base}_bitmap_data"
    else:
        var_name = "bitmap_data"

    with open(output_file, 'w', newline='\n') as f:
        f.write("#ifndef BITMAP_H\n")
        f.write("#define BITMAP_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"#define BITMAP_WIDTH {width}\n")
        f.write(f"#define BITMAP_HEIGHT {height}\n\n")
        f.write(f"const uint8_t {var_name}[{len(bitmap_data)}] = {{\n")

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

def png_to_bitmap_data(png_file):
    """Convert PNG to bitmap data array and return (filename_base, width, height, bitmap_data)."""
    img = Image.open(png_file)

    # Convert to RGB if necessary
    if img.mode != 'RGB':
        img = img.convert('RGB')

    width, height = img.size
    pixels = img.load()

    # Convert RGB to 8-bit color (R3G3B2)
    bitmap_data = [width & 0xFF, height & 0xFF]
    for y in range(height):
        for x in range(width):
            r, g, b = pixels[x, y][:3]
            color_8bit = ((r >> 5) << 5) | ((g >> 5) << 2) | (b >> 6)
            bitmap_data.append(color_8bit)

    filename_base = os.path.splitext(os.path.basename(png_file))[0]
    filename_base = re.sub(r'[^a-zA-Z0-9_]', '_', filename_base)
    
    return filename_base, width, height, bitmap_data

def combine_bitmaps_to_file(png_files, output_file):
    """Combine multiple PNG images into a single C header file."""
    
    # Ensure output directory exists
    out_dir = os.path.dirname(output_file)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)

    # Convert all images
    bitmaps = []
    for png_file in png_files:
        filename_base, width, height, bitmap_data = png_to_bitmap_data(png_file)
        bitmaps.append((filename_base, width, height, bitmap_data))

    # Write combined header file
    with open(output_file, 'w', newline='\n') as f:
        f.write("#ifndef BITMAPS_H\n")
        f.write("#define BITMAPS_H\n\n")
        f.write("#include <stdint.h>\n\n")

        # Write all bitmap arrays
        for filename_base, width, height, bitmap_data in bitmaps:
            var_name = f"{filename_base}_bitmap_data"
            f.write(f"// {filename_base} ({width}x{height})\n")
            f.write(f"const uint8_t {var_name}[{len(bitmap_data)}] = {{\n")

            for i, byte in enumerate(bitmap_data):
                if i % 16 == 0:
                    f.write("    ")
                f.write(f"0x{byte:02X}")
                if i < len(bitmap_data) - 1:
                    f.write(", ")
                if (i + 1) % 16 == 0:
                    f.write("\n")

            f.write("\n};\n\n")

        f.write("#endif // BITMAPS_H\n")

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
        png_to_c_bitmap(png_path, save_path, use_filename=True)
        show_file_contents_dialog(parent, save_path)
    except Exception as e:
        QMessageBox.critical(parent, "Error", f"Failed to convert image:\n{e}")

def batch_convert(parent=None):
    """Select multiple PNG files and convert them all to C headers."""
    png_paths, _ = QFileDialog.getOpenFileNames(
        parent,
        "Select PNG images to convert",
        "",
        "PNG Images (*.png *.PNG);;All Files (*)"
    )
    if not png_paths:
        return

    output_dir = QFileDialog.getExistingDirectory(
        parent,
        "Select output directory for .h files",
        os.path.dirname(png_paths[0]) if png_paths else ""
    )
    if not output_dir:
        return

    success_count = 0
    error_list = []

    for png_path in png_paths:
        try:
            filename_base = os.path.splitext(os.path.basename(png_path))[0]
            output_file = os.path.join(output_dir, f"{filename_base}.h")
            png_to_c_bitmap(png_path, output_file, use_filename=True)
            success_count += 1
        except Exception as e:
            error_list.append(f"{os.path.basename(png_path)}: {e}")

    message = f"Successfully converted {success_count} out of {len(png_paths)} images.\n"
    if error_list:
        message += "\nErrors:\n" + "\n".join(error_list)
    
    QMessageBox.information(parent, "Batch Conversion Complete", message)

def combine_multiple_bitmaps(parent=None):
    """Select multiple PNG files and combine them into a single C header file."""
    png_paths, _ = QFileDialog.getOpenFileNames(
        parent,
        "Select PNG images to combine",
        "",
        "PNG Images (*.png *.PNG);;All Files (*)"
    )
    if not png_paths:
        return

    default_name = "Bitmaps.h"
    default_dir = os.path.dirname(png_paths[0]) if png_paths else ""

    save_path, _ = QFileDialog.getSaveFileName(
        parent,
        "Save combined header file",
        os.path.join(default_dir, default_name),
        "C Header Files (*.h);;All Files (*)"
    )
    if not save_path:
        return

    try:
        combine_bitmaps_to_file(png_paths, save_path)
        show_file_contents_dialog(parent, save_path)
    except Exception as e:
        QMessageBox.critical(parent, "Error", f"Failed to combine images:\n{e}")

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
    dlg.setWindowTitle("PNG to Bitmap Converter")
    dlg.resize(320, 220)
    layout = QVBoxLayout(dlg)

    btn_convert = QPushButton("Convert Single PNG to .h", dlg)
    btn_batch = QPushButton("Batch Convert Multiple PNGs", dlg)
    btn_combine = QPushButton("Combine Multiple PNGs into 1 .h", dlg)
    btn_load = QPushButton("Load and show .h bitmap", dlg)
    btn_exit = QPushButton("Exit", dlg)

    layout.addWidget(btn_convert)
    layout.addWidget(btn_batch)
    layout.addWidget(btn_combine)
    layout.addWidget(btn_load)
    layout.addWidget(btn_exit)

    btn_convert.clicked.connect(lambda: pick_and_convert(dlg))
    btn_batch.clicked.connect(lambda: batch_convert(dlg))
    btn_combine.clicked.connect(lambda: combine_multiple_bitmaps(dlg))
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