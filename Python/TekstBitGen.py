import os
import sys
from PIL import Image, ImageDraw, ImageFont
from PyQt6.QtWidgets import (
    QApplication,
    QFileDialog,
    QMessageBox,
    QDialog,
    QVBoxLayout,
    QHBoxLayout,
    QTextEdit,
    QPushButton,
    QLabel,
    QSpinBox,
    QComboBox,
    QFormLayout,
    QCheckBox,
    QScrollArea,
    QListWidget,
    QListWidgetItem,
    QTableWidget,
    QTableWidgetItem,
    QHeaderView,
    QInputDialog,
)
from PyQt6.QtGui import QPixmap, QImage, QCursor
from PyQt6.QtCore import Qt
import re
from PyQt6.QtGui import QClipboard, QPainter, QColor, QMouseEvent

# Custom clickable label class
class ClickableLabel(QLabel):
    """QLabel that emits a signal when clicked."""
    def __init__(self, text="", parent=None):
        super().__init__(text, parent)
        self.clicked_callback = None
    
    def mousePressEvent(self, event):
        if self.clicked_callback:
            self.clicked_callback()

class BitmapEditor(QLabel):
    """Widget for editing bitmap pixels by clicking."""
    def __init__(self, width, height, bitmap_data, parent=None):
        super().__init__(parent)
        self.width = width
        self.height = height
        self.bitmap_data = bytearray(bitmap_data)
        self.cell_size = 20
        self.setMinimumSize(width * self.cell_size, height * self.cell_size)
        self.draw_bitmap()
    
    def draw_bitmap(self):
        """Draw the current bitmap state."""
        img = Image.new('RGB', (self.width, self.height))
        pixels = img.load()
        
        data_index = 0
        bytes_per_row = (self.width + 7) // 8
        
        for y in range(self.height):
            row_start = y * bytes_per_row
            for byte_in_row in range(bytes_per_row):
                byte_index = row_start + byte_in_row
                if byte_index >= len(self.bitmap_data):
                    break
                
                byte = self.bitmap_data[byte_index]
                
                for bit in range(8):
                    x = byte_in_row * 8 + bit
                    if x < self.width:
                        bit_value = (byte >> (7 - bit)) & 1
                        pixels[x, y] = (255, 255, 255) if bit_value else (0, 0, 0)
        
        # Scale for display
        img_scaled = img.resize((self.width * self.cell_size, self.height * self.cell_size), Image.NEAREST)
        b = img_scaled.tobytes()
        qimg = QImage(b, img_scaled.width, img_scaled.height, img_scaled.width * 3, QImage.Format.Format_RGB888)
        self.setPixmap(QPixmap.fromImage(qimg))
    
    def mousePressEvent(self, event):
        """Toggle pixel on click."""
        # Get the pixmap and calculate the scale based on actual display size
        if self.pixmap() is None:
            return
        
        pixmap = self.pixmap()
        # Scale factor: pixmap size / original bitmap size
        scale_x = pixmap.width() / self.width
        scale_y = pixmap.height() / self.height
        
        # Convert click position to bitmap coordinates
        x = int(event.position().x() / scale_x)
        y = int(event.position().y() / scale_y)
        
        if 0 <= x < self.width and 0 <= y < self.height:
            # Calculate byte index for row-by-row layout
            # Each row has ceil(width/8) bytes
            bytes_per_row = (self.width + 7) // 8
            byte_row_start = y * bytes_per_row
            byte_in_row = x // 8
            byte_index = byte_row_start + byte_in_row
            bit_index = 7 - (x % 8)
            
            # Toggle bit
            if byte_index < len(self.bitmap_data):
                self.bitmap_data[byte_index] ^= (1 << bit_index)
                self.draw_bitmap()

def show_error_dialog(parent, title, message):
    """Show error dialog with a copy button to copy message to clipboard."""
    dlg = QMessageBox(parent)
    dlg.setWindowTitle(title)
    dlg.setText(message)
    dlg.setIcon(QMessageBox.Icon.Critical)
    
    # Add copy button
    copy_btn = dlg.addButton("Copy Error", QMessageBox.ButtonRole.ActionRole)
    dlg.addButton(QMessageBox.StandardButton.Ok)
    
    # Connect copy button
    copy_btn.clicked.connect(lambda: QApplication.clipboard().setText(message))
    
    dlg.exec()

def text_to_bitmap(text, font_size=20, font_name=None, padding=10):
    """Convert text to a monochrome bitmap array (1 bit per pixel).
    
    Returns (text_sanitized, width, height, bitmap_data)
    Each byte contains 8 pixels (MSB to LSB)
    """
    
    # Try to load system font, fallback to default
    try:
        if font_name:
            font = ImageFont.truetype(font_name, font_size)
        else:
            # Try common system fonts
            font = ImageFont.load_default()
    except Exception:
        font = ImageFont.load_default()
    
    # Create temporary image to measure text
    temp_img = Image.new('RGB', (1, 1))
    temp_draw = ImageDraw.Draw(temp_img)
    bbox = temp_draw.textbbox((0, 0), text, font=font)
    text_width = bbox[2] - bbox[0]
    text_height = bbox[3] - bbox[1]
    
    # Store bbox offsets to properly position text with descenders
    bbox_left = bbox[0]
    bbox_top = bbox[1]
    
    # Add padding
    width = text_width + (padding * 2)
    height = text_height + (padding * 2)
    
    # Create actual image with black background (0)
    img = Image.new('1', (width, height), 0)  # 1-bit image, black background
    draw = ImageDraw.Draw(img)
    
    # Draw text in white (1), compensating for bbox offset
    draw.text((padding - bbox_left, padding - bbox_top), text, font=font, fill=1)
    
    # Convert to bitmap data (1 bit per pixel, packed into bytes)
    bitmap_data = [width & 0xFF, height & 0xFF]
    pixels = img.load()
    
    # Pack bits into bytes (8 pixels per byte, MSB first)
    for y in range(height):
        for x in range(0, width, 8):
            byte = 0
            for bit in range(8):
                if x + bit < width:
                    if pixels[x + bit, y]:
                        byte |= (1 << (7 - bit))  # MSB first
            bitmap_data.append(byte)
    
    # Sanitize text for variable name
    text_sanitized = re.sub(r'[^a-zA-Z0-9_]', '_', text)
    if text_sanitized and text_sanitized[0].isdigit():
        text_sanitized = f"txt_{text_sanitized}"
    
    return text_sanitized, width, height, bitmap_data

def text_to_c_bitmap(text, output_file, font_size=20, font_name=None, padding=10):
    """Generate monochrome text bitmap and save as C header file."""
    
    text_sanitized, width, height, bitmap_data = text_to_bitmap(
        text, font_size, font_name, padding
    )
    
    # Ensure output directory exists
    out_dir = os.path.dirname(output_file)
    if out_dir:
        os.makedirs(out_dir, exist_ok=True)
    
    var_name = f"{text_sanitized}_size{font_size}_bitmap"
    
    with open(output_file, 'w', newline='\n') as f:
        f.write("#ifndef TEXT_BITMAP_H\n")
        f.write("#define TEXT_BITMAP_H\n\n")
        f.write("#include <stdint.h>\n\n")
        f.write(f"// Text: \"{text}\" (Size: {font_size}, {width}x{height})\n")
        f.write(f"// 1-bit monochrome bitmap (1=white/foreground, 0=black/background)\n")
        f.write(f"#define TEXT_WIDTH {width}\n")
        f.write(f"#define TEXT_HEIGHT {height}\n\n")
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
        f.write("#endif // TEXT_BITMAP_H\n")

def bitmap_to_image(width, height, bitmap_data):
    """Convert 1-bit monochrome bitmap data back to PIL Image (RGB for display)."""
    img = Image.new('RGB', (width, height))
    pixels = img.load()
    
    # Skip first 2 bytes (width and height)
    data_index = 2
    
    for y in range(height):
        for x in range(0, width, 8):
            if data_index >= len(bitmap_data):
                break
            
            byte = bitmap_data[data_index]
            data_index += 1
            
            # Unpack bits from byte (MSB first)
            for bit in range(8):
                if x + bit < width:
                    if byte & (1 << (7 - bit)):
                        pixels[x + bit, y] = (255, 255, 255)  # White for 1
                    else:
                        pixels[x + bit, y] = (0, 0, 0)  # Black for 0
    
    return img

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

def generate_ascii_charset(parent=None):
    """Generate all printable ASCII characters as individual bitmaps."""
    dlg = QDialog(parent)
    dlg.setWindowTitle("Generate ASCII Character Set")
    dlg.resize(400, 180)
    layout = QFormLayout(dlg)
    
    # Font size
    size_spinner = QSpinBox(dlg)
    size_spinner.setMinimum(8)
    size_spinner.setMaximum(200)
    size_spinner.setValue(16)
    layout.addRow("Font Size:", size_spinner)
    
    # Padding
    padding_spinner = QSpinBox(dlg)
    padding_spinner.setMinimum(0)
    padding_spinner.setMaximum(50)
    padding_spinner.setValue(5)
    layout.addRow("Padding:", padding_spinner)
    
    # Buttons
    button_layout = QHBoxLayout()
    btn_generate = QPushButton("Generate", dlg)
    btn_cancel = QPushButton("Cancel", dlg)
    button_layout.addWidget(btn_generate)
    button_layout.addWidget(btn_cancel)
    layout.addRow(button_layout)
    
    def on_generate():
        output_dir = QFileDialog.getExistingDirectory(
            dlg,
            "Select output directory"
        )
        if not output_dir:
            return
        
        try:
            font_size = size_spinner.value()
            padding = padding_spinner.value()
            
            # Generate all printable ASCII characters (32-126)
            all_bitmaps = []
            all_data = []
            char_index_table = []
            offset = 0
            
            for ascii_code in range(32, 127):
                char = chr(ascii_code)
                text_sanitized, width, height, bitmap_data = text_to_bitmap(
                    char, font_size, padding=padding
                )
                all_bitmaps.append((char, ascii_code, text_sanitized, width, height, bitmap_data))
                
                # Store offset and size for this character
                char_index_table.append((ascii_code, width, height, offset))
                
                # Add bitmap data to single array (skip first 2 bytes which are width/height)
                all_data.extend(bitmap_data[2:])
                offset += len(bitmap_data[2:])
            
            # Save all characters in a single array file
            output_file = os.path.join(output_dir, f"ascii_charset_size{font_size}.h")
            with open(output_file, 'w', newline='\n') as f:
                f.write("#ifndef ASCII_CHARSET_H\n")
                f.write("#define ASCII_CHARSET_H\n\n")
                f.write("#include <stdint.h>\n\n")
                f.write(f"// ASCII Character Set (Size: {font_size}px)\n")
                f.write(f"// All characters (ASCII 32-126) stored in single array\n")
                f.write(f"// Total characters: {len(all_bitmaps)}\n\n")
                
                # Write character index table
                f.write("// Character Index Table: [ASCII code, width, height, offset in data array]\n")
                f.write(f"const uint16_t ascii_char_index[{len(char_index_table)}][4] = {{\n")
                for i, (ascii_code, width, height, offset) in enumerate(char_index_table):
                    f.write(f"    {{{ascii_code}, {width}, {height}, {offset}}}")
                    if i < len(char_index_table) - 1:
                        f.write(",")
                    # Add character as comment
                    if ascii_code == 32:
                        f.write(f"  // SPACE\n")
                    else:
                        f.write(f"  // '{chr(ascii_code)}'\n")
                f.write("};\n\n")
                
                # Write all bitmap data in single array
                f.write(f"// Bitmap data for all characters (1-bit monochrome)\n")
                f.write(f"const uint8_t ascii_bitmap_data[{len(all_data)}] = {{\n")
                
                for i, byte in enumerate(all_data):
                    if i % 16 == 0:
                        f.write("    ")
                    f.write(f"0x{byte:02X}")
                    if i < len(all_data) - 1:
                        f.write(", ")
                    if (i + 1) % 16 == 0:
                        f.write("\n")
                
                f.write("\n};\n\n")
                f.write("#endif // ASCII_CHARSET_H\n")
            
            QMessageBox.information(dlg, "Success", f"Generated ASCII charset with {len(all_bitmaps)} characters\nFile: ascii_charset_size{font_size}.h")
            dlg.accept()
        except Exception as e:
            show_error_dialog(dlg, "Error", f"Failed to generate ASCII charset:\n{e}")
    
    btn_generate.clicked.connect(on_generate)
    btn_cancel.clicked.connect(dlg.reject)
    
    dlg.exec()

def view_charset_file(parent=None, file_path=None):
    """Load and display a charset file with all characters."""
    if not file_path:
        header_path, _ = QFileDialog.getOpenFileName(
            parent,
            "Select ASCII charset header file",
            "",
            "C Header Files (*.h);;All Files (*)"
        )
        if not header_path:
            return
    else:
        header_path = file_path
    
    try:
        # Read the header file
        with open(header_path, 'r') as f:
            content = f.read()
        
        # Extract the character index table (flexible pattern for different naming)
        index_match = re.search(r'const\s+uint16_t\s+(\w+)\s*\[\s*(\d+)\s*\]\s*\[\s*4\s*\]\s*=\s*\{(.*?)\};', content, re.DOTALL)
        if not index_match:
            show_error_dialog(parent, "Error", "Could not find character index table in file")
            return
        
        num_chars = int(index_match.group(2))
        index_data = index_match.group(3)
        
        # Parse the index table
        char_specs = []
        lines = index_data.split('\n')
        for line in lines:
            # Extract numbers from lines like: {32, 8, 16, 0},  // SPACE
            match = re.search(r'\{(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\}', line)
            if match:
                ascii_code = int(match.group(1))
                width = int(match.group(2))
                height = int(match.group(3))
                offset = int(match.group(4))
                char_specs.append((ascii_code, width, height, offset))
        
        # Extract bitmap data (flexible pattern for different naming)
        bitmap_match = re.search(r'const\s+uint8_t\s+(\w+)\s*\[\s*(\d+)\s*\]\s*=\s*\{(.*?)\};', content, re.DOTALL)
        if not bitmap_match:
            show_error_dialog(parent, "Error", "Could not find bitmap data in file")
            return
        
        hex_values = re.findall(r'0x([0-9A-Fa-f]{2})', bitmap_match.group(3))
        bitmap_data = [int(h, 16) for h in hex_values]
        
        # Get font size from filename
        filename = os.path.basename(header_path)
        size_match = re.search(r'size(\d+)', filename)
        font_size = int(size_match.group(1)) if size_match else 16
        
        # Create display window
        dlg = QDialog(parent)
        dlg.setWindowTitle(f"Charset: {os.path.basename(header_path)}")
        dlg.resize(1200, 700)
        layout = QVBoxLayout(dlg)
        
        # Info label
        info_label = QLabel(
            f"Font Size: {font_size}px | Characters: {num_chars} | Bitmap Data Size: {len(bitmap_data)} bytes",
            dlg
        )
        layout.addWidget(info_label)
        
        # Create toolbar with editing options
        toolbar_layout = QHBoxLayout()
        
        # Search/filter box
        search_label = QLabel("Filter Characters:", dlg)
        search_input = QTextEdit(dlg)
        search_input.setMaximumHeight(30)
        search_input.setPlaceholderText("Enter characters or ASCII codes (e.g., 'abc' or '65-67')")
        
        def filter_characters():
            """Filter and display only selected characters."""
            filter_text = search_input.toPlainText().strip()
            if not filter_text:
                # Show all characters
                filtered_specs = char_specs
            else:
                filtered_specs = []
                # Parse filter - can be characters or ASCII ranges
                for spec in char_specs:
                    ascii_code = spec[0]
                    char = chr(ascii_code) if 32 <= ascii_code <= 126 else f"[{ascii_code}]"
                    
                    # Check if character matches
                    if char in filter_text:
                        filtered_specs.append(spec)
                    # Check if ASCII code matches
                    elif str(ascii_code) in filter_text:
                        filtered_specs.append(spec)
                    # Check for ranges like "65-67"
                    elif '-' in filter_text:
                        for range_str in filter_text.split(','):
                            if '-' in range_str:
                                try:
                                    start, end = map(int, range_str.strip().split('-'))
                                    if start <= ascii_code <= end:
                                        filtered_specs.append(spec)
                                        break
                                except:
                                    pass
            
            # Clear and rebuild character display
            while container_layout.count() > 0:
                container_layout.takeAt(0).widget()
            
            # Rebuild with filtered characters
            for ascii_code, width, height, offset in filtered_specs:
                # Create bitmap image
                # Calculate proper data size: bytes_per_row * height
                bytes_per_row = (width + 7) // 8
                data_size = bytes_per_row * height
                char_bitmap_data = bitmap_data[offset:offset + data_size]
                img = Image.new('RGB', (width, height))
                pixels = img.load()
                
                # Read bitmap row by row
                for y in range(height):
                    row_start = y * bytes_per_row
                    for byte_in_row in range(bytes_per_row):
                        byte_index = row_start + byte_in_row
                        if byte_index >= len(char_bitmap_data):
                            break
                        
                        byte = char_bitmap_data[byte_index]
                        
                        for bit in range(8):
                            x = byte_in_row * 8 + bit
                            if x < width:
                                if byte & (1 << (7 - bit)):
                                    pixels[x, y] = (255, 255, 255)
                                else:
                                    pixels[x, y] = (0, 0, 0)
                
                # Scale up for visibility
                scale = max(2, 24 // max(1, max(width, height)))
                img_scaled = img.resize((width * scale, height * scale), Image.NEAREST)
                
                # Convert to QImage
                b = img_scaled.tobytes()
                bytes_per_line = img_scaled.width * 3
                qimg = QImage(b, img_scaled.width, img_scaled.height, bytes_per_line, QImage.Format.Format_RGB888)
                
                # Create character row
                char_row = QHBoxLayout()
                
                # Character label
                clickable_label = ClickableLabel("[SPACE]" if ascii_code == 32 else (f"[{ascii_code}]" if ascii_code < 33 or ascii_code == 127 else f"'{chr(ascii_code)}'"), dlg)
                clickable_label.setMinimumWidth(50)
                clickable_label.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
                clickable_label.clicked_callback = lambda code=ascii_code, w=width, h=height, off=offset: show_enlarged_char(code, w, h, off)
                char_row.addWidget(clickable_label)
                
                # Character image (make it clickable too)
                clickable_pixmap = ClickableLabel("", dlg)
                clickable_pixmap.setPixmap(QPixmap.fromImage(qimg))
                clickable_pixmap.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
                clickable_pixmap.clicked_callback = lambda code=ascii_code, w=width, h=height, off=offset: show_enlarged_char(code, w, h, off)
                char_row.addWidget(clickable_pixmap)
                
                # Character info (make it clickable too)
                clickable_info = ClickableLabel(f"ASCII {ascii_code} | {width}x{height}", dlg)
                clickable_info.setMinimumWidth(120)
                clickable_info.setCursor(QCursor(Qt.CursorShape.PointingHandCursor))
                clickable_info.clicked_callback = lambda code=ascii_code, w=width, h=height, off=offset: show_enlarged_char(code, w, h, off)
                char_row.addWidget(clickable_info)
                
                char_row.addStretch()
                container_layout.addLayout(char_row)
            
            container_layout.addStretch()
        
        toolbar_layout.addWidget(search_label)
        toolbar_layout.addWidget(search_input)
        
        btn_filter = QPushButton("Filter", dlg)
        btn_filter.clicked.connect(filter_characters)
        toolbar_layout.addWidget(btn_filter)
        
        btn_show_all = QPushButton("Show All", dlg)
        btn_show_all.clicked.connect(lambda: (search_input.clear(), filter_characters()))
        toolbar_layout.addWidget(btn_show_all)
        
        layout.addLayout(toolbar_layout)
        
        # Create scrollable area for characters
        scroll = QScrollArea(dlg)
        scroll.setWidgetResizable(True)
        
        # Container for character grid
        container = QDialog()
        container_layout = QVBoxLayout(container)
        
        def show_enlarged_char(ascii_code, width, height, offset):
            """Show enlarged character in popup dialog."""
            # Extract this character's bitmap data
            bytes_per_row = (width + 7) // 8
            data_size = bytes_per_row * height
            char_bitmap_data = bitmap_data[offset:offset + data_size]
            
            # Convert to image
            img = Image.new('RGB', (width, height))
            pixels = img.load()
            
            # Read bitmap row by row
            for y in range(height):
                row_start = y * bytes_per_row
                for byte_in_row in range(bytes_per_row):
                    byte_index = row_start + byte_in_row
                    if byte_index >= len(char_bitmap_data):
                        break
                    
                    byte = char_bitmap_data[byte_index]
                    
                    for bit in range(8):
                        x = byte_in_row * 8 + bit
                        if x < width:
                            if byte & (1 << (7 - bit)):
                                pixels[x, y] = (255, 255, 255)
                            else:
                                pixels[x, y] = (0, 0, 0)
            
            # Scale up for visibility (much larger)
            scale = 10
            img_scaled = img.resize((width * scale, height * scale), Image.NEAREST)
            
            # Convert to QImage
            b = img_scaled.tobytes()
            bytes_per_line = img_scaled.width * 3
            qimg = QImage(b, img_scaled.width, img_scaled.height, bytes_per_line, QImage.Format.Format_RGB888)
            
            # Create popup dialog
            popup = QDialog(dlg)
            popup.setWindowTitle(f"Character: ASCII {ascii_code}")
            popup.resize(width * 25 + 100, height * 25 + 200)
            
            popup_layout = QVBoxLayout(popup)
            
            # Character info
            if ascii_code == 32:
                char_display = "[SPACE]"
            elif ascii_code < 33 or ascii_code == 127:
                char_display = f"[{ascii_code}]"
            else:
                char_display = f"'{chr(ascii_code)}'"
            
            info_label = QLabel(
                f"Character: {char_display} (ASCII {ascii_code})\nSize: {width}x{height}",
                popup
            )
            popup_layout.addWidget(info_label)
            
            # Bitmap editor (clickable grid)
            bytes_per_row = (width + 7) // 8
            data_size = bytes_per_row * height
            char_bitmap_data = bitmap_data[offset:offset + data_size]
            editor = BitmapEditor(width, height, char_bitmap_data, popup)
            popup_layout.addWidget(editor)
            
            # Button layout
            btn_layout = QHBoxLayout()
            
            # Save button
            def save_changes():
                """Save edited bitmap back to main bitmap_data."""
                for i, byte_val in enumerate(editor.bitmap_data):
                    if offset + i < len(bitmap_data):
                        bitmap_data[offset + i] = byte_val
                QMessageBox.information(popup, "Success", "Character changes saved!")
                popup.accept()
                # Refresh the character grid display
                filter_characters()
            
            btn_save = QPushButton("Save Changes", popup)
            btn_save.clicked.connect(save_changes)
            btn_layout.addWidget(btn_save)
            
            # Clear button (make all white)
            def clear_char():
                """Clear all pixels."""
                for i in range(len(editor.bitmap_data)):
                    editor.bitmap_data[i] = 0
                editor.draw_bitmap()
            
            btn_clear = QPushButton("Clear", popup)
            btn_clear.clicked.connect(clear_char)
            btn_layout.addWidget(btn_clear)
            
            # Invert button
            def invert_char():
                """Invert all pixels."""
                for i in range(len(editor.bitmap_data)):
                    editor.bitmap_data[i] ^= 0xFF
                editor.draw_bitmap()
            
            btn_invert = QPushButton("Invert", popup)
            btn_invert.clicked.connect(invert_char)
            btn_layout.addWidget(btn_invert)
            
            # Close button
            btn_close = QPushButton("Close (No Save)", popup)
            btn_close.clicked.connect(popup.reject)
            btn_layout.addWidget(btn_close)
            
            popup_layout.addLayout(btn_layout)
            
            popup.exec()
        
        scroll.setWidget(container)
        layout.addWidget(scroll)
        
        # Initialize display with all characters
        filter_characters()
        
        # Export modified charset button
        def export_modified_charset():
            """Export the modified bitmap data to a new header file."""
            export_path, _ = QFileDialog.getSaveFileName(
                dlg,
                "Save Modified Charset",
                os.path.splitext(header_path)[0] + "_modified.h",
                "C Header Files (*.h)"
            )
            if not export_path:
                return
            
            try:
                # Create new header file with modified data
                with open(export_path, 'w') as f:
                    f.write("#ifndef ASCII_CHARSET_H\n")
                    f.write("#define ASCII_CHARSET_H\n\n")
                    f.write("#include <stdint.h>\n\n")
                    f.write(f"// Modified ASCII Character Set\n")
                    f.write(f"// {num_chars} characters\n\n")
                    
                    # Write index table
                    f.write(f"const uint16_t ascii_char_index[{num_chars}][4] = {{\n")
                    for i, (ascii_code, width, height, offset) in enumerate(char_specs):
                        f.write(f"    {{{ascii_code}, {width}, {height}, {offset}}}")
                        if ascii_code == 32:
                            f.write("  // SPACE")
                        elif ascii_code >= 32 and ascii_code < 127:
                            f.write(f"  // '{chr(ascii_code)}'")
                        else:
                            f.write(f"  // ASCII {ascii_code}")
                        if i < num_chars - 1:
                            f.write(",")
                        f.write("\n")
                    f.write("};\n\n")
                    
                    # Write bitmap data
                    f.write(f"const uint8_t ascii_bitmap_data[{len(bitmap_data)}] = {{\n")
                    for i, byte in enumerate(bitmap_data):
                        if i % 16 == 0:
                            f.write("    ")
                        f.write(f"0x{byte:02X}")
                        if i < len(bitmap_data) - 1:
                            f.write(", ")
                        if (i + 1) % 16 == 0:
                            f.write("\n")
                    if len(bitmap_data) % 16 != 0:
                        f.write("\n")
                    f.write("};\n\n")
                    f.write("#endif // ASCII_CHARSET_H\n")
                
                QMessageBox.information(dlg, "Success", f"Charset exported to:\n{export_path}")
            except Exception as e:
                show_error_dialog(dlg, "Export Error", f"Failed to export charset:\n{e}")
        
        btn_export = QPushButton("Export Modified Charset", dlg)
        btn_export.clicked.connect(export_modified_charset)
        layout.addWidget(btn_export)
        
        # Close button
        btn_close = QPushButton("Close", dlg)
        btn_close.clicked.connect(dlg.accept)
        layout.addWidget(btn_close)
        
        dlg.exec()
        
    except Exception as e:
        show_error_dialog(parent, "Error", f"Failed to load charset file:\n{e}")

def convert_ttf_to_charset(parent=None):
    """Convert a TTF font file to ASCII charset bitmap."""
    # Select TTF file
    ttf_path, _ = QFileDialog.getOpenFileName(
        parent,
        "Select TTF Font File",
        "",
        "TrueType Fonts (*.ttf);;All Files (*)"
    )
    if not ttf_path:
        return
    
    # Create dialog for font settings
    dlg = QDialog(parent)
    dlg.setWindowTitle("TTF to Charset Converter")
    dlg.resize(400, 180)
    layout = QVBoxLayout(dlg)
    
    # Font info
    font_name = os.path.basename(ttf_path)
    info_label = QLabel(f"Font: {font_name}", dlg)
    layout.addWidget(info_label)
    
    # Font size selector
    form_layout = QFormLayout()
    
    spin_size = QSpinBox(dlg)
    spin_size.setMinimum(6)
    spin_size.setMaximum(128)
    spin_size.setValue(16)
    form_layout.addRow("Font Size:", spin_size)
    
    # Padding selector
    spin_padding = QSpinBox(dlg)
    spin_padding.setMinimum(0)
    spin_padding.setMaximum(20)
    spin_padding.setValue(2)
    form_layout.addRow("Padding:", spin_padding)
    
    layout.addLayout(form_layout)
    
    # Buttons
    btn_layout = QHBoxLayout()
    btn_generate = QPushButton("Generate Charset", dlg)
    btn_cancel = QPushButton("Cancel", dlg)
    btn_layout.addWidget(btn_generate)
    btn_layout.addWidget(btn_cancel)
    layout.addLayout(btn_layout)
    
    def on_generate():
        font_size = spin_size.value()
        padding = spin_padding.value()
        
        try:
            # Get output directory
            output_dir = QFileDialog.getExistingDirectory(
                dlg,
                "Select Output Directory",
                ""
            )
            if not output_dir:
                return
            
            # Generate all ASCII characters using the TTF font
            all_bitmaps = []
            all_data = []
            char_index_table = []
            offset = 0
            
            for ascii_code in range(32, 127):
                char = chr(ascii_code)
                text_sanitized, width, height, bitmap_data = text_to_bitmap(
                    char, font_size, ttf_path, padding=padding
                )
                all_bitmaps.append((char, ascii_code, text_sanitized, width, height, bitmap_data))
                
                # Store offset and size for this character
                char_index_table.append((ascii_code, width, height, offset))
                
                # Add bitmap data to single array (skip first 2 bytes which are width/height)
                all_data.extend(bitmap_data[2:])
                offset += len(bitmap_data[2:])
            
            # Save charset file
            font_basename = os.path.splitext(os.path.basename(ttf_path))[0]
            output_file = os.path.join(output_dir, f"{font_basename}_size{font_size}.h")
            
            with open(output_file, 'w', newline='\n') as f:
                f.write("#ifndef ASCII_CHARSET_H\n")
                f.write("#define ASCII_CHARSET_H\n\n")
                f.write("#include <stdint.h>\n\n")
                f.write(f"// ASCII Character Set from {font_name} (Size: {font_size}px)\n")
                f.write(f"// All characters (ASCII 32-126) stored in single array\n")
                f.write(f"// Total characters: {len(all_bitmaps)}\n\n")
                
                # Write character index table
                f.write("// Character Index Table: [ASCII code, width, height, offset in data array]\n")
                f.write(f"const uint16_t ascii_char_index[{len(char_index_table)}][4] = {{\n")
                for i, (ascii_code, width, height, offset) in enumerate(char_index_table):
                    f.write(f"    {{{ascii_code}, {width}, {height}, {offset}}}")
                    if i < len(char_index_table) - 1:
                        f.write(",")
                    # Add character as comment
                    if ascii_code == 32:
                        f.write(f"  // SPACE\n")
                    else:
                        f.write(f"  // '{chr(ascii_code)}'\n")
                f.write("};\n\n")
                
                # Write all bitmap data in single array
                f.write(f"// Bitmap data for all characters (1-bit monochrome)\n")
                f.write(f"const uint8_t ascii_bitmap_data[{len(all_data)}] = {{\n")
                
                for i, byte in enumerate(all_data):
                    if i % 16 == 0:
                        f.write("    ")
                    f.write(f"0x{byte:02X}")
                    if i < len(all_data) - 1:
                        f.write(", ")
                    if (i + 1) % 16 == 0:
                        f.write("\n")
                if len(all_data) % 16 != 0:
                    f.write("\n")
                f.write("};\n\n")
                f.write("#endif // ASCII_CHARSET_H\n")
            
            QMessageBox.information(dlg, "Success", f"Generated charset from TTF with {len(all_bitmaps)} characters\nFile: {output_file}")
            dlg.accept()
        except Exception as e:
            show_error_dialog(dlg, "Error", f"Failed to generate charset from TTF:\n{e}")
    
    btn_generate.clicked.connect(on_generate)
    btn_cancel.clicked.connect(dlg.reject)
    
    dlg.exec()

def convert_ttf_folder_to_charset(parent=None):
    """Convert all TTF files in a folder to ASCII charset bitmaps."""
    # Select folder containing TTF files
    folder_path = QFileDialog.getExistingDirectory(
        parent,
        "Select Folder Containing TTF Files",
        ""
    )
    if not folder_path:
        return
    
    # Find all TTF files in the folder
    ttf_files = []
    for file in os.listdir(folder_path):
        if file.lower().endswith('.ttf'):
            ttf_files.append(os.path.join(folder_path, file))
    
    if not ttf_files:
        QMessageBox.warning(parent, "No TTF Files", f"No TTF files found in:\n{folder_path}")
        return
    
    # Create dialog for batch settings
    dlg = QDialog(parent)
    dlg.setWindowTitle("Batch TTF to Charset Converter")
    dlg.resize(450, 220)
    layout = QVBoxLayout(dlg)
    
    # Info label
    info_label = QLabel(f"Found {len(ttf_files)} TTF file(s) in folder", dlg)
    layout.addWidget(info_label)
    
    # File list (scrollable)
    file_list = QLabel("\n".join([os.path.basename(f) for f in ttf_files[:10]]), dlg)
    if len(ttf_files) > 10:
        file_list.setText(file_list.text() + f"\n... and {len(ttf_files) - 10} more")
    scroll = QScrollArea(dlg)
    scroll.setWidget(file_list)
    scroll.setMaximumHeight(100)
    layout.addWidget(scroll)
    
    # Font size selector
    form_layout = QFormLayout()
    
    spin_size = QSpinBox(dlg)
    spin_size.setMinimum(6)
    spin_size.setMaximum(128)
    spin_size.setValue(16)
    form_layout.addRow("Font Size:", spin_size)
    
    # Padding selector
    spin_padding = QSpinBox(dlg)
    spin_padding.setMinimum(0)
    spin_padding.setMaximum(20)
    spin_padding.setValue(2)
    form_layout.addRow("Padding:", spin_padding)
    
    layout.addLayout(form_layout)
    
    # Buttons
    btn_layout = QHBoxLayout()
    btn_generate = QPushButton("Convert All", dlg)
    btn_cancel = QPushButton("Cancel", dlg)
    btn_layout.addWidget(btn_generate)
    btn_layout.addWidget(btn_cancel)
    layout.addLayout(btn_layout)
    
    def on_generate_all():
        font_size = spin_size.value()
        padding = spin_padding.value()
        
        # Get output directory
        output_dir = QFileDialog.getExistingDirectory(
            dlg,
            "Select Output Directory for Charset Files",
            ""
        )
        if not output_dir:
            return
        
        try:
            successful = 0
            failed = []
            
            for ttf_path in ttf_files:
                try:
                    font_name = os.path.basename(ttf_path)
                    
                    # Load font
                    font = ImageFont.truetype(ttf_path, font_size)
                    
                    # Generate all ASCII characters
                    all_bitmaps = []
                    all_data = []
                    char_index_table = []
                    offset = 0
                    
                    for ascii_code in range(32, 127):
                        char = chr(ascii_code)
                        text_sanitized, width, height, bitmap_data = text_to_bitmap(
                            char, font_size, ttf_path, padding=padding
                        )
                        all_bitmaps.append((char, ascii_code, text_sanitized, width, height, bitmap_data))
                        
                        # Store offset and size for this character
                        char_index_table.append((ascii_code, width, height, offset))
                        
                        # Add bitmap data to single array (skip first 2 bytes which are width/height)
                        all_data.extend(bitmap_data[2:])
                        offset += len(bitmap_data[2:])
                    
                    # Save charset file
                    font_basename = os.path.splitext(os.path.basename(ttf_path))[0]
                    output_file = os.path.join(output_dir, f"{font_basename}_size{font_size}.h")
                    
                    with open(output_file, 'w', newline='\n') as f:
                        f.write("#ifndef ASCII_CHARSET_H\n")
                        f.write("#define ASCII_CHARSET_H\n\n")
                        f.write("#include <stdint.h>\n\n")
                        f.write(f"// ASCII Character Set from {font_name} (Size: {font_size}px)\n")
                        f.write(f"// All characters (ASCII 32-126) stored in single array\n")
                        f.write(f"// Total characters: {len(all_bitmaps)}\n\n")
                        
                        # Write character index table
                        f.write("// Character Index Table: [ASCII code, width, height, offset in data array]\n")
                        f.write(f"const uint16_t ascii_char_index[{len(char_index_table)}][4] = {{\n")
                        for i, (ascii_code, width, height, offset) in enumerate(char_index_table):
                            f.write(f"    {{{ascii_code}, {width}, {height}, {offset}}}")
                            if i < len(char_index_table) - 1:
                                f.write(",")
                            # Add character as comment
                            if ascii_code == 32:
                                f.write(f"  // SPACE\n")
                            else:
                                f.write(f"  // '{chr(ascii_code)}'\n")
                        f.write("};\n\n")
                        
                        # Write all bitmap data in single array
                        f.write(f"// Bitmap data for all characters (1-bit monochrome)\n")
                        f.write(f"const uint8_t ascii_bitmap_data[{len(all_data)}] = {{\n")
                        
                        for i, byte in enumerate(all_data):
                            if i % 16 == 0:
                                f.write("    ")
                            f.write(f"0x{byte:02X}")
                            if i < len(all_data) - 1:
                                f.write(", ")
                            if (i + 1) % 16 == 0:
                                f.write("\n")
                        if len(all_data) % 16 != 0:
                            f.write("\n")
                        f.write("};\n\n")
                        f.write("#endif // ASCII_CHARSET_H\n")
                    
                    successful += 1
                    
                except Exception as e:
                    failed.append((font_name, str(e)))
            
            # Show results
            result_msg = f"Successfully converted {successful} of {len(ttf_files)} font(s)\n"
            if failed:
                result_msg += f"\nFailed to convert:\n"
                for name, error in failed[:5]:  # Show first 5 failures
                    result_msg += f"- {name}: {error}\n"
                if len(failed) > 5:
                    result_msg += f"... and {len(failed) - 5} more"
            
            QMessageBox.information(dlg, "Batch Conversion Complete", result_msg)
            dlg.accept()
            
        except Exception as e:
            show_error_dialog(dlg, "Error", f"Batch conversion failed:\n{e}")
    
    btn_generate.clicked.connect(on_generate_all)
    btn_cancel.clicked.connect(dlg.reject)
    
    dlg.exec()

def organize_fonts_dialog(file_paths, parent=None):
    """Dialog to organize, reorder, and preview selected font files."""
    dlg = QDialog(parent)
    dlg.setWindowTitle("Organize Fonts")
    dlg.resize(600, 500)
    layout = QVBoxLayout(dlg)
    
    # Info label
    info_label = QLabel(f"Organize {len(file_paths)} selected font files:", dlg)
    layout.addWidget(info_label)
    
    # Table widget for fonts
    font_table = QTableWidget(dlg)
    font_table.setColumnCount(3)
    font_table.setHorizontalHeaderLabels(["Name", "Size (px)", "Path"])
    font_table.setSelectionBehavior(QTableWidget.SelectionBehavior.SelectRows)
    font_table.setSelectionMode(QTableWidget.SelectionMode.SingleSelection)
    font_table.setRowCount(len(file_paths))
    
    # Add files to table
    for row, file_path in enumerate(file_paths):
        filename = os.path.basename(file_path)
        # Try to get size from filename
        size_match = re.search(r'size(\d+)', filename)
        default_size = int(size_match.group(1)) if size_match else 16
        
        # Name column
        name_item = QTableWidgetItem(filename)
        name_item.setData(Qt.ItemDataRole.UserRole, file_path)  # Store full path
        font_table.setItem(row, 0, name_item)
        
        # Size column
        size_item = QTableWidgetItem(str(default_size))
        font_table.setItem(row, 1, size_item)
        
        # Path column
        path_item = QTableWidgetItem(file_path)
        path_item.setFlags(path_item.flags() & ~Qt.ItemFlag.ItemIsEditable)  # Read-only
        font_table.setItem(row, 2, path_item)
    
    # Adjust column widths
    font_table.horizontalHeader().setSectionResizeMode(0, QHeaderView.ResizeMode.Stretch)
    font_table.horizontalHeader().setSectionResizeMode(1, QHeaderView.ResizeMode.ResizeToContents)
    font_table.horizontalHeader().setSectionResizeMode(2, QHeaderView.ResizeMode.Stretch)
    
    layout.addWidget(font_table)
    
    # Control buttons
    btn_layout = QHBoxLayout()
    
    btn_move_up = QPushButton("↑ Move Up", dlg)
    btn_move_down = QPushButton("↓ Move Down", dlg)
    btn_rename = QPushButton("✎ Rename", dlg)
    btn_edit_size = QPushButton("📏 Edit Size", dlg)
    btn_remove = QPushButton("✕ Remove", dlg)
    btn_preview = QPushButton("👁 Preview", dlg)
    
    btn_layout.addWidget(btn_move_up)
    btn_layout.addWidget(btn_move_down)
    btn_layout.addWidget(btn_rename)
    btn_layout.addWidget(btn_edit_size)
    btn_layout.addWidget(btn_remove)
    btn_layout.addWidget(btn_preview)
    layout.addLayout(btn_layout)
    
    # Move up function
    def move_up():
        current_row = font_table.currentRow()
        if current_row > 0:
            # Swap rows
            for col in range(3):
                item_current = font_table.takeItem(current_row, col)
                item_above = font_table.takeItem(current_row - 1, col)
                font_table.setItem(current_row, col, item_above)
                font_table.setItem(current_row - 1, col, item_current)
            font_table.setCurrentCell(current_row - 1, 0)
    
    # Move down function
    def move_down():
        current_row = font_table.currentRow()
        if current_row < font_table.rowCount() - 1:
            # Swap rows
            for col in range(3):
                item_current = font_table.takeItem(current_row, col)
                item_below = font_table.takeItem(current_row + 1, col)
                font_table.setItem(current_row, col, item_below)
                font_table.setItem(current_row + 1, col, item_current)
            font_table.setCurrentCell(current_row + 1, 0)
    
    # Rename function
    def rename_item():
        current_row = font_table.currentRow()
        if current_row >= 0:
            name_item = font_table.item(current_row, 0)
            current_name = name_item.text()
            
            # Prompt for new name
            new_name, ok = QInputDialog.getText(
                dlg, 
                "Rename Font", 
                "Enter new name for this font:",
                text=current_name
            )
            
            if ok and new_name:
                name_item.setText(new_name)
    
    # Edit size function
    def edit_size():
        current_row = font_table.currentRow()
        if current_row >= 0:
            size_item = font_table.item(current_row, 1)
            current_size = int(size_item.text())
            
            # Prompt for new size
            new_size, ok = QInputDialog.getInt(
                dlg,
                "Edit Font Size",
                "Enter font size (px):",
                value=current_size,
                min=6,
                max=128
            )
            
            if ok:
                size_item.setText(str(new_size))
    
    # Remove function
    def remove_item():
        current_row = font_table.currentRow()
        if current_row >= 0:
            font_table.removeRow(current_row)
            if font_table.rowCount() == 0:
                QMessageBox.warning(dlg, "No Fonts", "At least one font must be selected.")
    
    # Preview function
    def preview_font():
        current_row = font_table.currentRow()
        if current_row >= 0:
            path_item = font_table.item(current_row, 2)
            file_path = path_item.text()
            view_charset_file(dlg, file_path)
    
    btn_move_up.clicked.connect(move_up)
    btn_move_down.clicked.connect(move_down)
    btn_rename.clicked.connect(rename_item)
    btn_edit_size.clicked.connect(edit_size)
    btn_remove.clicked.connect(remove_item)
    btn_preview.clicked.connect(preview_font)
    
    # Action buttons
    action_layout = QHBoxLayout()
    
    btn_combine = QPushButton("Combine Fonts", dlg)
    btn_cancel = QPushButton("Cancel", dlg)
    
    action_layout.addWidget(btn_combine)
    action_layout.addWidget(btn_cancel)
    layout.addLayout(action_layout)
    
    # Result list with custom names
    result_data = []
    
    def on_combine():
        if font_table.rowCount() == 0:
            QMessageBox.warning(dlg, "No Fonts", "At least one font must be selected.")
            return
        
        # Get ordered file paths, custom names, and sizes from table
        for row in range(font_table.rowCount()):
            name_item = font_table.item(row, 0)
            size_item = font_table.item(row, 1)
            path_item = font_table.item(row, 2)
            
            custom_name = name_item.text()
            custom_size = int(size_item.text())
            file_path = path_item.text()
            
            result_data.append((file_path, custom_name, custom_size))
        
        dlg.accept()
    
    btn_combine.clicked.connect(on_combine)
    btn_cancel.clicked.connect(dlg.reject)
    
    if dlg.exec() == QDialog.DialogCode.Accepted:
        return result_data
    return None

def combine_charset_files(parent=None):
    """Combine multiple charset files into a single header file."""
    # Select multiple charset files
    file_paths, _ = QFileDialog.getOpenFileNames(
        parent,
        "Select Charset Files to Combine",
        "",
        "C Header Files (*.h);;All Files (*)"
    )
    
    if not file_paths or len(file_paths) < 1:
        return
    
    # Show organize dialog
    organized_data = organize_fonts_dialog(file_paths, parent)
    if not organized_data:
        return
    
    try:
        combined_charsets = []
        
        # Read each charset file
        for file_path, custom_name, custom_size in organized_data:
            with open(file_path, 'r') as f:
                content = f.read()
            
            # Check if file has actual content
            if len(content.strip()) < 100:
                show_error_dialog(parent, "Error", f"File appears to be empty or incomplete:\n{file_path}\n\nPlease regenerate this charset file.")
                return
            
            # Use custom name and size
            font_name = custom_name
            font_size = custom_size
            
            # Extract character index table (flexible pattern for different naming)
            index_match = re.search(r'const\s+uint16_t\s+(\w+)\s*\[\s*(\d+)\s*\]\s*\[\s*4\s*\]\s*=\s*\{(.*?)\};', content, re.DOTALL)
            if not index_match:
                show_error_dialog(parent, "Error", f"Could not find character index table in:\n{file_path}\n\nThe file may be incomplete or corrupted.\nPlease regenerate this charset file.")
                return
            
            index_var_name = index_match.group(1)
            num_chars = int(index_match.group(2))
            index_data = index_match.group(3)
            
            # Parse the index table
            char_specs = []
            lines = index_data.split('\n')
            for line in lines:
                match = re.search(r'\{(\d+),\s*(\d+),\s*(\d+),\s*(\d+)\}', line)
                if match:
                    ascii_code = int(match.group(1))
                    width = int(match.group(2))
                    height = int(match.group(3))
                    offset = int(match.group(4))
                    char_specs.append((ascii_code, width, height, offset))
            
            # Extract bitmap data (flexible pattern for different naming)
            bitmap_match = re.search(r'const\s+uint8_t\s+(\w+)\s*\[\s*(\d+)\s*\]\s*=\s*\{(.*?)\};', content, re.DOTALL)
            if not bitmap_match:
                show_error_dialog(parent, "Error", f"Could not find bitmap data in:\n{file_path}\n\nThe file may be incomplete or corrupted.\nPlease regenerate this charset file.")
                return
            
            bitmap_var_name = bitmap_match.group(1)
            hex_values = re.findall(r'0x([0-9A-Fa-f]{2})', bitmap_match.group(3))
            bitmap_data = [int(h, 16) for h in hex_values]
            
            # Use custom size (already set above)
            combined_charsets.append({
                'name': font_name,
                'size': font_size,
                'num_chars': num_chars,
                'char_specs': char_specs,
                'bitmap_data': bitmap_data
            })
        
        # Get output file path
        output_path, _ = QFileDialog.getSaveFileName(
            parent,
            "Save Combined Charset File",
            "combined_charsets.h",
            "C Header Files (*.h)"
        )
        
        if not output_path:
            return
        
        # Write combined header file
        with open(output_path, 'w', newline='\n') as f:
            f.write("#ifndef COMBINED_CHARSETS_H\n")
            f.write("#define COMBINED_CHARSETS_H\n\n")
            f.write("#include <stdint.h>\n\n")
            f.write(f"// Combined Character Sets - {len(combined_charsets)} fonts\n\n")
            
            # Track used names to avoid duplicates
            used_names = {}
            safe_names = []
            
            # Generate unique safe names for each charset
            for charset in combined_charsets:
                font_name = charset['name']
                safe_name = re.sub(r'[^a-zA-Z0-9_]', '_', font_name)
                
                # Check for duplicate names
                if safe_name in used_names:
                    used_names[safe_name] += 1
                    safe_name = f"{safe_name}_{used_names[safe_name]}"
                else:
                    used_names[safe_name] = 1
                
                safe_names.append(safe_name)
            
            # Write each charset
            for idx, charset in enumerate(combined_charsets):
                font_name = charset['name']
                safe_name = safe_names[idx]
                
                f.write(f"// Font {idx + 1}: {font_name}\n")
                f.write(f"// Characters: {charset['num_chars']}, Data size: {len(charset['bitmap_data'])} bytes\n\n")
                
                # Write index table
                f.write(f"const uint16_t {safe_name}_index[{charset['num_chars']}][4] = {{\n")
                for i, (ascii_code, width, height, offset) in enumerate(charset['char_specs']):
                    f.write(f"    {{{ascii_code}, {width}, {height}, {offset}}}")
                    if i < charset['num_chars'] - 1:
                        f.write(",")
                    if ascii_code == 32:
                        f.write("  // SPACE")
                    elif ascii_code >= 32 and ascii_code < 127:
                        f.write(f"  // '{chr(ascii_code)}'")
                    else:
                        f.write(f"  // ASCII {ascii_code}")
                    f.write("\n")
                f.write("};\n\n")
                
                # Write bitmap data
                f.write(f"const uint8_t {safe_name}_data[{len(charset['bitmap_data'])}] = {{\n")
                for i, byte in enumerate(charset['bitmap_data']):
                    if i % 16 == 0:
                        f.write("    ")
                    f.write(f"0x{byte:02X}")
                    if i < len(charset['bitmap_data']) - 1:
                        f.write(", ")
                    if (i + 1) % 16 == 0:
                        f.write("\n")
                if len(charset['bitmap_data']) % 16 != 0:
                    f.write("\n")
                f.write("};\n\n")
            
            # Write lookup structure
            f.write("// Font lookup structure\n")
            f.write("typedef struct {\n")
            f.write("    const char* name;\n")
            f.write("    uint16_t size;\n")
            f.write("    uint16_t num_chars;\n")
            f.write("    const uint16_t (*index)[4];\n")
            f.write("    const uint8_t* data;\n")
            f.write("} FontInfo;\n\n")
            
            f.write(f"const FontInfo available_fonts[{len(combined_charsets)}] = {{\n")
            for idx, charset in enumerate(combined_charsets):
                font_name = charset['name']
                safe_name = safe_names[idx]
                f.write(f"    {{\"{font_name}\", {charset['size']}, {charset['num_chars']}, {safe_name}_index, {safe_name}_data}}")
                if idx < len(combined_charsets) - 1:
                    f.write(",")
                f.write("\n")
            f.write("};\n\n")
            
            f.write(f"#define NUM_FONTS {len(combined_charsets)}\n\n")
            f.write("#endif // COMBINED_CHARSETS_H\n")
        
        QMessageBox.information(parent, "Success", f"Combined {len(combined_charsets)} charset files\nOutput: {output_path}")
        
    except Exception as e:
        show_error_dialog(parent, "Error", f"Failed to combine charset files:\n{e}")

def main_menu():
    app = QApplication(sys.argv)
    
    dlg = QDialog()
    dlg.setWindowTitle("Text Bitmap Generator")
    dlg.resize(350, 260)
    layout = QVBoxLayout(dlg)
    
    btn_ascii = QPushButton("Generate ASCII Character Set", dlg)
    btn_ttf = QPushButton("Convert TTF Font to Charset", dlg)
    btn_ttf_folder = QPushButton("Convert TTF Folder to Charset", dlg)
    btn_combine = QPushButton("Combine Charset Files", dlg)
    btn_view = QPushButton("View Charset File", dlg)
    btn_exit = QPushButton("Exit", dlg)
    
    layout.addWidget(btn_ascii)
    layout.addWidget(btn_ttf)
    layout.addWidget(btn_ttf_folder)
    layout.addWidget(btn_combine)
    layout.addWidget(btn_view)
    layout.addWidget(btn_exit)
    
    btn_ascii.clicked.connect(lambda: generate_ascii_charset(dlg))
    btn_ttf.clicked.connect(lambda: convert_ttf_to_charset(dlg))
    btn_ttf_folder.clicked.connect(lambda: convert_ttf_folder_to_charset(dlg))
    btn_combine.clicked.connect(lambda: combine_charset_files(dlg))
    btn_view.clicked.connect(lambda: view_charset_file(dlg))
    btn_exit.clicked.connect(dlg.accept)
    
    dlg.exec()

if __name__ == "__main__":
    main_menu()
