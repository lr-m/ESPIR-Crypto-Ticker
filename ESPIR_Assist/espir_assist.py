import tkinter as tk
from tkinter import ttk
from tkinter.colorchooser import askcolor
from PIL import Image, ImageTk
from math import sqrt, floor
import os
import re
from tkinter.filedialog import askopenfilename, askdirectory

# Changes logo colour
def change_logo_colour():
    global logo_colour_565
    global logo_colour_888

    # Get colour from user
    colours = askcolor(title="Logo Colour")

    # Convert to rgb 565 and 888
    logo_colour_888 = hex_to_rgb_888(colours[1][1:])
    logo_colour_565 = hex_to_rgb_565(colours[1][1:])

    # Update label
    logo_label.configure(text=hex(logo_colour_565))

    # Refresh colour and bitmap gen
    generate_output()
    refresh_bitmaps()

# Changes bitmap background colour
def change_bg_colour():
    global background_colour_565
    global background_colour_888

    # Get colour from user
    colours = askcolor(title="Background Colour")

    # Convert to rgb 565 and 888
    background_colour_888 = hex_to_rgb_888(colours[1][1:])
    background_colour_565 = hex_to_rgb_565(colours[1][1:])

    # Update label
    bg_label.configure(text=hex(background_colour_565))

    # Refresh colour and bitmap gen
    generate_output()
    refresh_bitmaps()

def hex_to_rgb_888(hex):
    # Convert to RGB
    return tuple(int(hex[i:i+2], 16) for i in (0, 2, 4))

# Converts hex to rgb 5:6:5 packed
def hex_to_rgb_565(hex):
    # Convert to RGB
    rgb888 = hex_to_rgb_888(hex)

    # Shift to get 5:6:5 RGB
    return ((rgb888[0] & 0b11111000) << 8) + \
        ((rgb888[1] & 0b11111100) << 3) + \
        ((rgb888[2] & 0b11111000) >> 3)

# Generate colour.h output
def generate_output():
    # Set output text
    output_copy.configure(text='#define ' + new_code_entry.get() + \
    '_LOGO_COLOUR ' + hex(logo_colour_565) + '\n#define ' + \
    new_code_entry.get() + '_BACKGROUND_COLOUR ' + hex(background_colour_565))

# Copies value in colour output to clipboard
def colour_h_clipboard():
    generate_output() # Update output
    root.clipboard_append(output_copy["text"])
    root.update()

# Copies value in bitmap output to clipboard
def bitmap_clipboard():
    refresh_bitmaps() # Update bitmaps first
    root.clipboard_append(bitmap_output["text"])
    root.update()

# Refresh the preview and new bitmaps
def refresh_bitmaps():
    global logo_filename

    # If no logo input, ignore
    if (logo_filename == ""):
        return

    # Copy the original logo into 2 new images
    logo_resized = Image.new("RGBA", (40,40))
    logo_preview = Image.new("RGBA", (40,40))
    logo_resized.paste(logo_original)
    logo_preview.paste(logo_original)

    # Used to store bitmap output
    output = "  "
    output_index = 0

    byteIndex = 7
    number = 0

    # If logo has a single colour channel (alpha for black and white), convert 
    # to RGBA
    if (type(logo_resized.getpixel((0,0))) is int):
        logo_resized = Image.open(logo_filename, 'r').resize((40,40)).convert("RGBA")
        for i in range(40**2):
            x = i%40
            y = floor(i/40)

            logo_resized.putpixel((x, y), (logo_resized.getpixel((x,y))[3], \
            logo_resized.getpixel((x,y))[3], logo_resized.getpixel((x,y))[3], 255))    

    for i in range(40**2):
        x = i%40
        y = floor(i/40)
        rgb = logo_resized.getpixel((x, y))

        # Get brightness value of pixel
        brightness_val = sqrt((0.299*rgb[0])**2 + (0.587*rgb[1])**2 + (0.114*rgb[2])**2 )/170.5 * 255

        # Check against cutoff
        if (((brightness_val < alpha_cutoff.get()) and (colour_invert.get() == 0)) or \
                ((not brightness_val < alpha_cutoff.get()) and (colour_invert.get() == 1))):
            number += 2**byteIndex;
            logo_resized.putpixel((x, y), (255, 255, 255, 255))
            logo_preview.putpixel((x, y), (logo_colour_888[0], logo_colour_888[1], logo_colour_888[2], 255))
        else:
            logo_resized.putpixel((x, y), (0, 0, 0, 255))
            logo_preview.putpixel((x, y), (background_colour_888[0], background_colour_888[1], background_colour_888[2], 255))

        byteIndex-=1;

        if (((i != 0) and (((i+1)%40 == 0) or (i == (40**2)-1)))):
            byteIndex = -1

        # Append hex data to output
        if (byteIndex < 0):
            hex_val_str = str(hex(number))[2:]

            if (len(hex_val_str) == 1):
                hex_val_str = '0' + hex_val_str

            hex_val_str = '0x' + hex_val_str

            output = output + hex_val_str + ", "

            output_index+=1

            if (output_index >= 16):
                output += "\n  "
                output_index = 0

            number = 0
            byteIndex = 7

    # Add data to bitmap output
    bitmap_output.configure(text = \
        "const unsigned char CODE_logo [] PROGMEM = {".replace("CODE", new_code_entry.get()) \
        + '\n' + output[:len(output)-2] + "\n};")

    # Update bitmaps/previews
    updated_new=ImageTk.PhotoImage(logo_resized)
    logo_resize_img.configure(image=updated_new)
    logo_resize_img.image = updated_new

    updated_preview = ImageTk.PhotoImage(logo_preview)
    logo_preview_img.configure(image=updated_preview)
    logo_preview_img.image = updated_preview

# Refresh bitmaps when slider value changes
def slider_changed(event):
    refresh_bitmaps()

# Function called when data is to be saved to sketch
def replace_sequence():
    # Check that there are no empty inputs
    if (new_code_entry.get() == "" or new_id_entry.get() == "" \
            or old_code_entry.get() == "" or ticker_directory_entry.get() == "" \
            or library_directory_entry.get() == ""):
        result_label.config(text= "Missing Input", fg='red')
        return

    # Make sure all outputs have been generated
    generate_output()
    refresh_bitmaps()

    library_path = library_directory_entry.get()
    ticker_path = ticker_directory_entry.get()

    # Either entry ends in desired entries, or the directory is in provided directory

    # Handle ticker
    if (os.path.exists(ticker_directory_entry.get())):
        print("\nValid Sketch Path")

    # Check that the path is valid
    if ('ticker' in os.listdir(ticker_directory_entry.get()) and \
            not ticker_directory_entry.get().endswith('ticker')):
        print('ticker found (subdirectory)')
        ticker_path = ticker_path + '\\ticker'
    elif (ticker_directory_entry.get().endswith('ticker')):
        print('ticker found (directory)')

    # Replace the bitmap in bitmaps.c
    print("\nModifying bitmaps.c")

    # Load data
    bitmaps_c = open(ticker_path + '\\bitmaps.c', 'r+')
    bitmaps_c_data = bitmaps_c.read()
    bitmaps_c.close()

    # Use regex to find replacing bitmap
    if (len(re.findall('const unsigned char ' +  old_code_entry.get() + \
            '_logo \[\] PROGMEM = {[0-9|a-f|\n|x| |,]*};', bitmaps_c_data)) == 0):
        result_label.config(text= "Failed in bitmap.c", fg='red')
        return
    print("Replacing: " + re.findall('const unsigned char ' +  \
        old_code_entry.get() + '_logo \[\] PROGMEM = {[0-9|a-f|\n|x| |,]*};', \
        bitmaps_c_data)[0])

    new_data = re.sub('const unsigned char ' +  old_code_entry.get() + \
        '_logo \[\] PROGMEM = {[0-9|a-f|\n|x| |,]*};', bitmap_output["text"], \
        bitmaps_c_data, re.DOTALL)

    print("Success")

    # Replace data in ticker.ino
    print("\nModifying ticker.ino...")
    ticker_ino = open(ticker_path + '\\ticker.ino', 'r+')
    ticker_ino_data = ticker_ino.read()
    ticker_ino.close()

    # Replace the extern bitmap 
    if (len(re.findall('extern unsigned char ' +  old_code_entry.get() + \
            '_logo\[\]', ticker_ino_data)) == 0):
        result_label.config(text= "Failed in ticker.ino (1)", fg='red')
        return
    
    print("Replacing: " + re.findall('extern unsigned char ' +  \
        old_code_entry.get() + '_logo\[\]', ticker_ino_data)[0])

    ticker_data = re.sub('extern unsigned char ' +  old_code_entry.get() + \
        '_logo\[\]', 'extern unsigned char ' +  new_code_entry.get() + \
        '_logo[]', ticker_ino_data)

    # Replace the coin definition
    accent_colour = new_code_entry.get() + '_LOGO_COLOUR'
    if (accent_options.get() == 'Background'):
        accent_colour = new_code_entry.get() + '_BACKGROUND_COLOUR'

    if (len(re.findall(" *COIN\(\"" + old_code_entry.get() + ".*", ticker_data)) == 0):
        result_label.config(text= "Failed in ticker.ino (2)", fg='red')
        return

    print("Replacing: " + re.findall(" *COIN\(\"" + old_code_entry.get() + \
        ".*", ticker_data)[0])

    final_ticker_data = re.sub(" *COIN\(\"" + old_code_entry.get() + ".*", \
        "    COIN(\"" + new_code_entry.get() + '\", \"' + new_id_entry.get() + \
        '\", ' + new_code_entry.get() + '_logo, ' + new_code_entry.get() + \
        '_BACKGROUND_COLOUR, ' + new_code_entry.get() + '_LOGO_COLOUR, ' + \
        accent_colour + ', 0, value_drawer);', ticker_data)

    # Save all the data back to files once all checks pass
    bitmaps_c_write = open(ticker_path + '\\bitmaps.c', 'w')
    bitmaps_c_write.write(new_data)
    bitmaps_c_write.close()

    ticker_ino_write = open(ticker_path + '\\ticker.ino', 'w')
    ticker_ino_write.write(final_ticker_data)
    ticker_ino_write.close()

    # Handle library
    if (os.path.exists(library_directory_entry.get())):
        print("\nValid Library Path")

    # Check that the path is valid
    if ('ST7735_Crypto_Ticker' in os.listdir(library_directory_entry.get()) and \
            not library_directory_entry.get().endswith('ST7735_Crypto_Ticker')):
        print('ST7735_Crypto_Ticker found (subdirectory)')
        library_path = library_path + '\ST7735_Crypto_Ticker'
    elif (ticker_directory_entry.get().endswith('ST7735_Crypto_Ticker')):
        print('ST7735_Crypto_Ticker found (directory)')

    # Open colours.h and append new colours to file
    print("\nModifying Colours.h...")
    colours_h = open(library_path + '\Colours.h', 'a')
    colours_h.write('#define ' + new_code_entry.get() + '_LOGO_COLOUR ' + \
        hex(logo_colour_565) + '\n#define ' + new_code_entry.get() + \
        '_BACKGROUND_COLOUR ' + hex(background_colour_565) + '\n')
    colours_h.close()
    print("Success")

    result_label.config(text= "Success", fg='green')

# Use dialogue box to get location of intended logo
def get_logo_filename():
    global logo_filename
    global logo_original

    # show an "Open" dialog box and return the path to the selected file
    logo_filename = askopenfilename() 

    # Update images to show new logo
    logo_original = Image.open(logo_filename, 'r').resize((40,40))
    original_tk = ImageTk.PhotoImage(logo_original)
    logo_orig_img.configure(image=original_tk)
    logo_orig_img.image = original_tk

    refresh_bitmaps()

# Get location of ticker directory from user
def get_ticker_directory():
    ticker_directory_entry.delete(0, tk.END)
    directory = askdirectory()
    ticker_directory_entry.insert(0, directory)

# Get location of library directory from user
def get_library_directory():
    library_directory_entry.delete(0, tk.END)
    directory = askdirectory()
    library_directory_entry.insert(0, directory)

logo_filename = "" # Path location of logo

# Colours
logo_colour_565 = 0x0000
background_colour_565 = 0x0000

logo_colour_888 = [0, 0, 0]
background_colour_888 = [0, 0, 0]

root = tk.Tk()
root.title('ESPIR Helper')
root.geometry('1280x720')
root.configure(bg='black')

# Variables for options
alpha_cutoff = tk.DoubleVar()
colour_invert = tk.IntVar()

# LEFT SIDE
left_frame = tk.Frame(root, bg='black', padx=25)

# Crypto details frame
details_frame = tk.Frame(left_frame, bg='black')

details_title = tk.Label(details_frame, text="New Coin Details", \
    font=("Arial", 25), bg='black', fg='white')
details_title.grid(row=0,column=0,columnspan=4)

code_frame = tk.Frame(details_frame, bg='black')

code_label = tk.Label(code_frame, text = "Enter Coin Code to Add", bg='black', \
    fg='white')  
code_label.pack(expand=True, side=tk.TOP)  

new_code_canvas = tk.Canvas(code_frame, width = 180, height = 40, bg='black', \
    highlightthickness=0)
new_code_canvas.pack(expand=True, side=tk.TOP)
new_code_entry = tk.Entry (code_frame) 
new_code_canvas.create_window(90, 20, window=new_code_entry)

code_frame.grid(column=0, row=1, padx=10, pady=10)

id_frame = tk.Frame(details_frame, bg='black')

code_label = tk.Label(id_frame, text = "Enter CoinGecko ID", bg='black', fg='white')  
code_label.pack(expand=True, side=tk.TOP)  

new_id_canvas = tk.Canvas(id_frame, width = 180, height = 40, bg='black', \
    highlightthickness=0)
new_id_canvas.pack(expand=True, side=tk.TOP)
new_id_entry = tk.Entry (id_frame) 
new_id_canvas.create_window(90, 20, window=new_id_entry)

id_frame.grid(column=2, row=1, padx=10, pady=10)

fg_button = tk.Button(
    details_frame,
    text='Select Logo PNG',
    command=get_logo_filename, 
    bg='green', 
    fg='white', 
    activeforeground='white', 
    activebackground='red', width=20).grid(column=3, row=1, padx=20, pady=10)

details_frame.pack(side=tk.TOP)
   
# Colour_h frame
colour_h_frame = tk.Frame(left_frame, bg='black')
colour_h_frame.pack()

colour_title = tk.Label(colour_h_frame, text="Colours.h", font=("Arial", 25), \
    bg='black', fg='white')
colour_title.grid(row=0,column=0,columnspan=5)

# Logo colour frame
logo_frame = tk.Frame(colour_h_frame, bg='black')

logo_label = tk.Label(logo_frame, text = "0x0", bg='black', fg='white')  
logo_label.pack(expand=True, side=tk.BOTTOM)  

fg_button = tk.Button(
    logo_frame,
    text='Select Logo Colour',
    command=change_logo_colour,bg='green', fg='white', activeforeground='white', \
        activebackground='red').pack()

logo_frame.grid(column=1, row=1, pady=5);

# Background colour frame
bg_frame = tk.Frame(colour_h_frame, bg='black')

bg_label = tk.Label(bg_frame, text = "0x0", bg='black', fg='white')  
bg_label.pack(expand=True, side=tk.BOTTOM)  

bg_button = tk.Button(
    bg_frame,
    text='Select Background Colour',
    command=change_bg_colour,bg='green', fg='white', activeforeground='white', \
        activebackground='red').pack()

bg_frame.grid(column=1, row=2, pady=5)

# Output label
output_frame = tk.Frame(colour_h_frame, bg='black')

output_copy = tk.Label(output_frame, text = "",justify=tk.LEFT, bg="white", \
    width=40, height=2)
output_copy.pack(side = tk.BOTTOM, fill='both')

generate_button = tk.Button(
    output_frame,
    text='Generate Output',
    command=generate_output,bg='green', fg='white', activeforeground='white', \
        activebackground='red').pack(side = tk.LEFT, fill='both', anchor='nw', \
        pady=10)

clipboard_colour_copy = tk.Button(
    output_frame,
    text='Copy',
    command=colour_h_clipboard, bg='green', fg='white', activeforeground='white', \
        activebackground='red').pack(fill='both', anchor='ne',side = tk.RIGHT, \
        pady=10)

output_frame.grid(column=4, row=1, padx=5, pady=20, rowspan=2, sticky='n')

# Accent colour
select_frame = tk.Frame(colour_h_frame, bg='black')
accent_options = tk.StringVar(root)
accent_options.set("Logo") # default value

accent_label = tk.Label(select_frame, text = "Accent Colour:", bg='black', \
    fg='white')  
accent_label.pack(expand=True, side=tk.TOP)  

w = tk.OptionMenu(select_frame, accent_options, "Logo", "Background")
w.config(bg='green', fg='white', highlightthickness=0, width=15, \
    activebackground="red", activeforeground="white")
w["menu"].config(bg="green", fg='white')
w.pack(fill='both')
select_frame.grid(column=3, row=1, rowspan=2, padx=30, pady=10, sticky='n')


# bitmap_h frame
bitmap_frame = tk.Frame(left_frame, bg='black');
bitmap_frame.pack()

bitmap_title = tk.Label(bitmap_frame, font=("Arial", 25), text='bitmaps.c', \
    bg='black', fg='white')
bitmap_title.grid(row=0, column=0, columnspan=5)

#logo_original = Image.open(logo_filename, 'r')
logo_original = Image.new("RGBA", (40,40))
logo_resized = logo_original.resize((40,40)) # new width & height
logo_preview = logo_original.resize((40,40)) # new width & height

for i in range(40):
    for j in range(40):
        logo_original.putpixel((i, j), (0, 0, 0, 255))
        logo_resized.putpixel((i, j), (0, 0, 0, 255))
        logo_preview.putpixel((i, j), (0, 0, 0, 255))

logo_orig_tk=ImageTk.PhotoImage(logo_original.resize((40,40)))
logo_new_tk=ImageTk.PhotoImage(logo_resized)
logo_preview_tk=ImageTk.PhotoImage(logo_preview)

tk.Label(bitmap_frame, text="Brightness Cut Off", bg='black', \
    fg='white').grid(row=1, column=0, padx=25)
tk.Label(bitmap_frame, text="Original:", bg='black', \
    fg='white').grid(row=1, column=1)
tk.Label(bitmap_frame, text="Bitmap:", bg='black', \
    fg='white').grid(row=1, column=2)
tk.Label(bitmap_frame, text="Preview:", bg='black', \
    fg='white').grid(row=1, column=3)

logo_orig_img = tk.Label(bitmap_frame, image=logo_orig_tk)
logo_orig_img.grid(column=1, row=2)

logo_resize_img = tk.Label(bitmap_frame, image=logo_new_tk)
logo_resize_img.grid(column=2, row=2)

logo_preview_img = tk.Label(bitmap_frame, image=logo_preview_tk)
logo_preview_img.grid(column=3, row=2)

config_frame = tk.Frame(bitmap_frame, bg='black')

slider = tk.Scale(
    config_frame,
    from_=0,
    to=255,
    orient='horizontal',
    command=slider_changed,
    variable=alpha_cutoff,
    bg='black',
    fg='white',
    highlightthickness=0,
    troughcolor='green'
).pack(side=tk.TOP)

c1 = tk.Checkbutton(config_frame, text='Invert Output Colours',\
    variable=colour_invert, onvalue=1, offvalue=0, command=refresh_bitmaps, \
    bg='black', fg='white', activebackground='black', activeforeground='white', \
    selectcolor='green').pack(side=tk.BOTTOM)

config_frame.grid(row=2, column=0)

bitmap_output_frame = tk.Frame(bitmap_frame, bg='black')
bitmap_output = tk.Label(bitmap_output_frame, text = " ",justify=tk.LEFT, \
    bg="white", width=80, height=15)
bitmap_output.pack()
bitmap_output_frame.grid(row = 4, column = 0, columnspan=5, pady=10)

clipboard_bitmap_copy = tk.Button(
    bitmap_frame,
    text='Copy',
    command=bitmap_clipboard,bg='green', fg='white', activeforeground='white', \
        activebackground='red').grid(row=2, column=4, sticky='se')

left_frame.pack(side=tk.LEFT, padx=25)

# RIGHT SIDE
right_frame = tk.Frame(root, bg='black')

# Logo at top right
espir = Image.open('espir_logo.png', 'r').resize((550, 200))
espir_tk=ImageTk.PhotoImage(espir)

espir_orig_img = tk.Label(right_frame, image=espir_tk, bg='black')
espir_orig_img.pack(side=tk.TOP)  

# Replacing details frame
replacing_details_frame = tk.Frame(right_frame, bg='black')

old_details_title = tk.Label(replacing_details_frame, \
    text="Replacing Coin Details", font=("Arial", 25), bg='black', fg='white')
old_details_title.grid(row=0,column=0,columnspan=1)

old_code_frame = tk.Frame(replacing_details_frame, bg='black')

old_code_label = tk.Label(old_code_frame, text = "Enter Coin Code to Replace", \
    bg='black', fg='white')  
old_code_label.pack(expand=True, side=tk.TOP)  

old_code_canvas = tk.Canvas(old_code_frame, width = 200, height = 40, \
    bg='black', highlightthickness=0)
old_code_canvas.pack(expand=True, side=tk.TOP)
old_code_entry = tk.Entry (old_code_frame) 
old_code_canvas.create_window(100, 20, window=old_code_entry)

old_code_frame.grid(column=0, row=1, padx=10, pady=20)

replacing_details_frame.pack(side=tk.TOP)

# Directory location frame
dir_location_frame = tk.Frame(right_frame, bg='black')

dir_location_title = tk.Label(dir_location_frame, text="Directories", \
    font=("Arial", 25), bg='black', fg='white')
dir_location_title.grid(row=0,column=0,columnspan=2)

# ST7735_Crypto_Ticker library
library_directory_frame = tk.Frame(dir_location_frame, bg='black')

library_directory_button = tk.Button(
    library_directory_frame,
    text='Select \'ST7735_Crypto_Ticker\' Directory/Folder',
    command=get_library_directory,bg='green', fg='white', activeforeground='white', \
        activebackground='red')
library_directory_button.pack(expand=True, side=tk.TOP)  

library_directory_canvas = tk.Canvas(library_directory_frame, width = 500, \
    height = 40, bg='black', highlightthickness=0)
library_directory_canvas.pack(expand=True, side=tk.TOP)
library_directory_entry = tk.Entry(library_directory_frame, width=70) 
library_directory_canvas.create_window(250, 20, window=library_directory_entry)

library_directory_frame.grid(column=0, row=1, padx=10, pady=10)

# Ticker directory
ticker_directory_frame = tk.Frame(dir_location_frame, bg='black')

ticker_directory_button = tk.Button(
    ticker_directory_frame,
    text='Select \'ticker\' Directory/Folder',
    command=get_ticker_directory,bg='green', fg='white', activeforeground='white', \
        activebackground='red')
ticker_directory_button.pack(expand=True, side=tk.TOP)  

ticker_directory_canvas = tk.Canvas(ticker_directory_frame, width = 500, \
    height = 40, bg='black', highlightthickness=0)
ticker_directory_canvas.pack(expand=True, side=tk.TOP)
ticker_directory_entry = tk.Entry (ticker_directory_frame, width=70) 
ticker_directory_canvas.create_window(250, 20, window=ticker_directory_entry)

ticker_directory_frame.grid(column=0, row=2, padx=10, pady=10)

result_label = tk.Label(ticker_directory_frame, text = "", bg='black', fg='white')  
result_label.pack(expand=True, side=tk.BOTTOM)  

replace_button = tk.Button(
    ticker_directory_frame,
    text='REPLACE',
    command=replace_sequence, width=25,
    bg='green', fg='white', activeforeground='white', activebackground='red')
replace_button.pack(side = tk.BOTTOM, pady=10)

dir_location_frame.pack(side=tk.TOP)

right_frame.pack(side = tk.RIGHT)

root.mainloop()