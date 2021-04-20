import tkinter.messagebox
import subprocess
import tkinter as tk
from tkinter import filedialog
from tkinter.filedialog import *
from tkinter import ttk
from pygments import lex
from pygments.lexers.c_cpp import CLexer, CppLexer
import os
import webbrowser
import signal
import Pmw


global open_status_name
open_status_name = False
global saved_file
saved_file = True
global selected
selected = False
global file_name
file_name = "fn"
global file_name_path
file_name_path = "fnp"
global file_type
file_type = "ft"
global file_location
file_location = "fl"
global process
process = subprocess.Popen(['dir'], shell=True)
global breakpoint_list
breakpoint_list = []
global debug_output
debug_output = []
global last_breakpoint
last_breakpoint = -1


def new_file(a):
    global saved_file
    global open_status_name
    saved_file = False
    open_status_name = False
    if my_text.get(1.0, 'end-1c') == "":
        my_text.delete(1.0, 'end-1c')
        root.title('New File - TextPad!')
        status_bar.config(text="New File    ")
    else:
        answer = tkinter.messagebox.askyesno(title="Warning", message="Do you want to save the file first?")
        if answer:
            save_file(False)
            my_text.delete(1.0, 'end-1c')
            root.title('New File - TextPad!')
            status_bar.config(text="New File    ")
        else:
            my_text.delete("1.0", tk.END)
            root.title('New File - TextPad!')
            status_bar.config(text="New File    ")
        redraw()


def open_file(a):
    global file_name
    global file_location
    global saved_file
    global file_type
    global file_name_path
    if my_text.get(1.0, 'end-1c') != "":
        if saved_file:
            pass
        else:
            answer = tkinter.messagebox.askyesno(title="Warning", message="Do you want to save this file?")
            if answer:
                save_file(False)
                return
    my_text.delete(1.0, 'end-1c')
    text_file = filedialog.askopenfilename(initialdir="C:", title="Open File", filetypes=(("C Files", "*.c"), ("C++ Files", "*.cpp")))

    if text_file:
        global open_status_name
        open_status_name = text_file
    else:
        return

    name = text_file
    saved_file = True
    name_list = name.split("/")
    name_split = name.split(".")
    file_name_path = name_list[-1]
    name_path_split = file_name_path.split(".")
    file_name = name_path_split[0]
    file_type = name_split[-1]
    name_list.remove(file_name_path)
    file_location = "\\".join(name_list)
    status_bar.config(text=f'{name}    ')
    root.title(f'{file_name_path} - TextPad!')

    text_file = open(text_file, 'r')
    stuff = text_file.read()
    my_text.insert(tk.END, stuff)
    text_file.close()
    redraw()
    highlight_text()


def save_file(a):
    global open_status_name
    global file_location
    global file_name
    global saved_file
    if open_status_name:
        text_file = open(open_status_name, 'w')
        text_file.write(my_text.get(1.0, 'end-1c'))
        text_file.close()
        status_bar.config(text=f'Saved: {file_location + file_name_path}')
        saved_file = True
        my_text.edit_modified(False)
        highlight_text()
    else:
        save_as(False)


def save_as(a):
    global file_name
    global file_location
    global saved_file
    global file_type
    text_file = filedialog.asksaveasfilename(defaultextension=".*", initialdir="C:", title="Save File", filetypes=(("C Files", "*.c"), ("C++ Files", "*.cpp")))
    if text_file:
        name = text_file
        saved_file = True
        my_text.edit_modified(False)
        name_list = name.split("/")
        name_split = name.split(".")
        file_name_path = name_list[-1]
        name_path_split = file_name_path.split(".")
        file_name = name_path_split[0]
        file_type = name_split[-1]
        file_location = name_list[0]
        status_bar.config(text=f'{name}    ')
        root.title(f'{file_name_path} - TextPad!')

        text_file = open(text_file, 'w')
        text_file.write(my_text.get(1.0, 'end-1c'))
        text_file.close()
        redraw()
        highlight_text()


def exit_app():
    if saved_file:
        if my_text.edit_modified() == 0:
            root.destroy()
            return
    answer = tkinter.messagebox.askyesnocancel(title="Warning", message="Do you want to save this file?")
    if answer == True:
        save_file(False)
    elif answer == False:
        root.destroy()
    return


def cut_text(a):
    global selected
    if a:
        selected = root.clipboard_get()
    else:
        if my_text.selection_get():
            selected = my_text.selection_get()
            my_text.delete("sel.first", "sel.last")
            root.clipboard_clear()
            root.clipboard_append(selected)


def copy_text(a):
    global selected
    if a:
        selected = root.clipboard_get()
    if my_text.selection_get():
        selected = my_text.selection_get()
        root.clipboard_clear()
        root.clipboard_append(selected)


def paste_text(a):
    global selected
    if a:
        selected = root.clipboard_get()
    else:
        if selected:
            position = my_text.index(tk.INSERT)
            my_text.insert(position, selected)


def find_text(a):
    find_win = tkinter.Toplevel()
    find_win.wm_title("Find")
    find_win.geometry("300x80")
    find_win.resizable(False, False)
    label1 = tk.Label(find_win, text="Find Text:", height=0, width=7)
    label1.pack(side=tk.LEFT, anchor=tk.N, padx=10, pady=10)
    entry1 = tk.Entry(find_win, width=40)
    entry1.pack(fill=tk.X, anchor=tk.N, padx=10, pady=10)
    button2 = tk.Button(find_win, text="Cancel", command=lambda: cancel_search())
    button2.pack(side=tk.RIGHT, padx=5, pady=5)
    button1 = tk.Button(find_win, text="Find Next", command=lambda: search_text())
    button1.pack(side=tk.RIGHT, padx=5, pady=5)
    find_win.bind('<Return>', lambda event: search_text())
    find_win.bind('<Escape>', lambda event: cancel_search())
    find_win.protocol("WM_DELETE_WINDOW", lambda: cancel_search())

    def cancel_search():
        my_text.tag_remove('found', '1.0', 'end-1c')
        find_win.destroy()

    def search_text():
        length = tkinter.IntVar()
        my_text.tag_remove('found', '1.0', 'end-1c')
        s = entry1.get()
        if s:
            idx = '1.0'
            while 1:
                idx = my_text.search(s, idx, nocase=1, stopindex=tk.END, count=length)
                if not idx:
                    break

                last_idx = f'{idx}+{length.get()}c'
                #last_idx = '%s+%dc' % (idx, len(s))

                my_text.tag_add('found', idx, last_idx)
                idx = last_idx
                my_text.tag_configure('found', background='yellow')
                my_text.see(last_idx)
        entry1.focus_set()
        if not my_text.tag_ranges('found'):
            tkinter.messagebox.showwarning(title="Warning", message="Not Found!")


def about_us():
    about_win = tkinter.Toplevel()
    about_win.wm_title("About Digital MCU Simulator")
    about_win.geometry("350x250")
    about_win.attributes("-toolwindow", True)
    label1 = tk.Label(about_win, text="Texas A&M University\nVersion 1.0.0\n© 2020 All Rights Reserved\n\nDigital MCU Simulator is an application that combines an IDE,\n a Microcontroller Simulator, a Waveform, \nand a Hardware Communication System. \n\nThis application is meant for educational use only.\n\nCreated by: Bunnarak Theng\n            Anvesh Kandi\n            Jonathan Howard\n            Jose Santos")
    label1.pack(side=tk.LEFT, padx=10, pady=10)


def compile(a):
    global file_location    
    global file_type
    global file_name
    global file_name_path
    global saved_file
    global process
    compile_output = ""
    result = ""
    if (file_name == "fn") and (file_name_path == "fnp") and (file_type == "ft") and (file_location == "fl"):
        saved_file = False
    if saved_file:
        check = check_for_compiler()
        if check:
            os.chdir(r"{}".format(file_location))
            f = open("Makefile", "w")
            if file_type.lower() == "c":
                f.write("PORT=\\\\\\\\.\\\\GLOBALROOT\\\\Device\\\\USBSER000\n"
                        "MCU=atmega328p\n"
                        "CFLAGS=-c -Os -Wall -DF_CPU=16000000L\n"
                        "CC=avr-gcc\n"
                        "OBJCOPY=avr-objcopy\n\n"
                        f"TARGET={file_name}\n\n"
                        "$(TARGET).hex: $(TARGET).elf\n\t"
                        "$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex\n\n"
                        "$(TARGET).elf: $(TARGET).o\n\t"
                        "$(CC) -mmcu=$(MCU) $(TARGET).o -o $(TARGET).elf\n\n"
                        "$(TARGET).o: $(TARGET).c\n\t"
                        "$(CC) $(CFLAGS) -mmcu=$(MCU) $(TARGET).c\n\n"
                        "clean:\n\t"
                        "rm -f *.o *.hex *.obj *.hex")
                f.close()
                p = subprocess.Popen(['make'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
                result = p.communicate()[0].decode("utf-8")
                if result.find("error") == -1:
                    compile_output = result
                    try:
                        f = open(file_name + ".hex", "r")
                        s = f.read()
                    except (OSError, IOError) as e:
                        compile_output += '\n' + e
                    finally:
                        compile_output += '\n' + s
                        f.close()
                else:
                    compile_output = "Error: \n\n" + result
            elif file_type.lower() == "cpp":
                f.write("PORT=\\\\\\\\.\\\\GLOBALROOT\\\\Device\\\\USBSER000\n"
                        "MCU=atmega328p\n"
                        "CFLAGS=-c -Os -Wall -DF_CPU=16000000L\n"
                        "CC=avr-g++\n"
                        "OBJCOPY=avr-objcopy\n\n"
                        f"TARGET={file_name}\n\n"
                        "$(TARGET).hex: $(TARGET).elf\n\t"
                        "$(OBJCOPY) -O ihex -R .eeprom $(TARGET).elf $(TARGET).hex\n\n"
                        "$(TARGET).elf: $(TARGET).o\n\t"
                        "$(CC) -mmcu=$(MCU) $(TARGET).o -o $(TARGET).elf\n\n"
                        "$(TARGET).o: $(TARGET).cpp\n\t"
                        "$(CC) $(CFLAGS) -mmcu=$(MCU) $(TARGET).cpp\n\n"
                        "clean:\n\t"
                        "rm -f *.o *.hex *.obj *.hex")
                f.close()
                p = subprocess.Popen(['make'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
                result = p.communicate()[0].decode("utf-8")
                if result.find("error") == -1:
                    compile_output = result
                    try:
                        f = open(file_name + ".hex", "r")
                        s = f.read()
                    except (OSError, IOError) as e:
                        compile_output += '\n' + e
                    finally:
                        compile_output += '\n' + s
                        f.close()
                else:
                    compile_output = "Error: \n\n" + result
            else:
                pass

            my_text2.configure(state="normal")
            my_text2.delete(1.0, 'end-1c')
            my_text2.insert(tk.INSERT, os.getcwd() + "\n" + compile_output)
            my_text2.configure(state="disabled")
        else:
            pass
    else:
        answer = tkinter.messagebox.askyesno(title="Warning", message="The file needs to be saved first before running. Do you want to save the file?")
        if answer:
            save_file(False)
        else:
            pass

"""def compile(a):
    global file_location
    global file_type
    global file_name
    global file_name_path
    global saved_file
    global process
    compile_output = ""
    error = ""
    print(file_location)
    print(file_name_path)
    print(file_name)
    print(file_type)
    if (file_name == "fn") and (file_name_path == "fnp") and (file_type == "ft") and (file_location == "fl"):
        saved_file = False
    if saved_file:
        check = check_for_compiler()
        if check:
            os.chdir(r"{}".format(file_location))
            if file_type.lower() == "cpp":
                p = subprocess.Popen(['g++', '-o', file_name + '.exe', file_name + '.cpp'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
                error = p.communicate()[0].decode("utf-8")
                if error == "":
                    p = subprocess.Popen([file_name + '.exe'], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
                    compile_output = p.communicate()[0].decode("utf-8")
                else:
                    compile_output = error
            elif file_type.lower() == "c":
                p = subprocess.Popen(['gcc', '-o', file_name + '.exe', file_name + '.c'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
                error = p.communicate()[0].decode("utf-8")
                if error == "":
                    p = subprocess.Popen([file_name + '.exe'], stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
                    compile_output = p.communicate()[0].decode("utf-8")
                else:
                    compile_output = error
            else:
                pass
            my_text2.configure(state="normal")
            my_text2.delete(1.0, 'end-1c')
            my_text2.insert(INSERT, os.getcwd() + "\n" + compile_output)
            my_text2.configure(state="disabled")
        else:
            pass
    else:
        answer = tkinter.messagebox.askyesno(title="Warning", message="The file needs to be saved first before running. Do you want to save the file?")
        if answer:
            save_file(False)
        else:
            pass"""


def check_for_compiler():
    global file_type
    s_to_find = "Copyright"
    s = ""
    if file_type.lower() == 'c' or file_type.lower() == 'cpp':
        if file_type.lower() == 'c':
            p = subprocess.Popen(['g++', '--version'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
            s = p.communicate()[0].decode("utf-8")
        elif file_type.lower() == 'cpp':
            p = subprocess.Popen(['gcc', '--version'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.PIPE)
            s = p.communicate()[0].decode("utf-8")
        if s.find(s_to_find) == -1:
            answer = tkinter.messagebox.askyesno(title="Warning", message="No C/C++ compiler can be found. Please install a C/C++ compiler first before running again.\n\nDo you want to install Cygwin?")
            if answer:
                webbrowser.open("https://cygwin.com/install.html")
            else:
                pass
        else:
            return True
    return False


def stop(p):
    print(p)
    os.kill(p.pid, signal.CTRL_C_EVENT)


def upload(a):
    print("Hello")


def start_debug(a):
    global file_type
    global file_name
    global file_location
    global saved_file
    global process
    global breakpoint_list
    global debug_output
    global last_breakpoint
    error = ""
    output = ""
    bp_location = []
    if len(breakpoint_list) == 0:
        compile(a)
        return
    if (file_name == "fn") and (file_name_path == "fnp") and (file_type == "ft") and (file_location == "fl"):
        saved_file = False
    if saved_file:
        check = check_for_compiler()
        if check:
            os.chdir(r"{}".format(file_location))
            if file_type.lower() == "cpp":
                process = subprocess.Popen(['g++', '-Wall', '-g3', file_name + '.cpp'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
                error = process.communicate()[0].decode("utf-8")
                if error == "":
                    process = subprocess.Popen(['gdb', 'a.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
                    debug_output.append(get_output(process, 16))
                    for i in breakpoint_list:
                        s = "break " + i + "\n"
                        process.stdin.write(s.encode("utf-8"))
                        process.stdin.flush()
                        m = get_output(process, 1)
                        j = m.find("line ")
                        bp_location.append(int(m[j + 5]))
                        debug_output.append(m)
                    process.stdin.write("run\n".encode("utf-8"))
                    process.stdin.flush()
                    debug_output.append(get_output(process, 5))
                    output = ''.join(debug_output)
                    last_breakpoint = min(bp_location)
                    my_text.tag_add("breakpoint", str(last_breakpoint) + ".0", str(last_breakpoint) + ".999")
                    my_text.tag_configure("breakpoint", background="red", foreground="black")
                    sketchMenu.entryconfigure(6, state='normal')
                    sketchMenu.entryconfigure(7, state='normal')
                    sketchMenu.entryconfigure(8, state='normal')
                else:
                    output = error
            elif file_type.lower() == "c":
                process = subprocess.Popen(['gcc', '-Wall', '-g3', file_name + '.c'], shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE, stderr=subprocess.STDOUT)
                error = process.communicate()[0].decode("utf-8")
                if error == "":
                    process = subprocess.Popen(['gdb', 'a.exe'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, shell=True)
                    debug_output.append(get_output(process, 16))
                    for i in breakpoint_list:
                        s = "break " + i + "\n"
                        process.stdin.write(s.encode("utf-8"))
                        process.stdin.flush()
                        m = get_output(process, 1)
                        j = m.find("line ")
                        bp_location.append(int(m[j + 5]))
                        debug_output.append(m)
                    process.stdin.write("run\n".encode("utf-8"))
                    process.stdin.flush()
                    debug_output.append(get_output(process, 5))
                    output = ''.join(debug_output)
                    last_breakpoint = min(bp_location)
                    my_text.tag_add("breakpoint", str(last_breakpoint) + ".0", str(last_breakpoint) + ".999")
                    my_text.tag_configure("breakpoint", background="red", foreground="black")
                    sketchMenu.entryconfigure(6, state='normal')
                    sketchMenu.entryconfigure(7, state='normal')
                    sketchMenu.entryconfigure(8, state='normal')
                else:
                    output = error
            else:
                pass
    else:
        answer = tkinter.messagebox.askyesno(title="Warning", message="The file needs to be saved first before running. Do you want to save the file?")
        if answer:
            save_file(False)
        else:
            pass
    my_text3.configure(state="normal")
    my_text3.delete(1.0, 'end-1c')
    my_text3.insert(tk.INSERT, os.getcwd() + "\n" + output)
    my_text3.configure(state="disabled")


def get_output(p, t):
    output = []
    while p.poll() is None:
        line = p.stdout.readline()
        output.append(line.decode("utf-8"))
        if len(output) == t:
            break
    return ''.join(output)


def step_debug(a):
    global process
    global debug_output
    global last_breakpoint
    j = []
    bp = "0"
    process.stdin.write("step\n".encode("utf-8"))
    process.stdin.flush()
    m = get_output(process, 1)
    if m[7] == '\t':
        bp = m[6]
    else:
        if m[8] == '\t':
            bp = m[6] + m[7]
        else:
            if m[9] == '\t':
                bp = m[6] + m[7] + m[8]
    my_text.tag_remove("breakpoint", str(last_breakpoint) + ".0", str(last_breakpoint) + ".999")
    last_breakpoint = bp
    my_text.tag_add("breakpoint", bp + ".0", bp + ".999")
    my_text.tag_configure("breakpoint", background="red", foreground="black")
    debug_output.append(m)
    s = ''.join(debug_output)
    my_text3.configure(state="normal")
    my_text3.delete(1.0, 'end-1c')
    my_text3.insert(tk.INSERT, os.getcwd() + "\n" + s)
    my_text3.configure(state="disabled")


def continue_debug(a):
    global process
    global last_breakpoint
    global debug_output
    process.stdin.write("continue\n".encode("utf-8"))
    process.stdin.flush()
    my_text.tag_remove("breakpoint", str(last_breakpoint) + ".0", str(last_breakpoint) + ".999")
    while process.poll() is None:
        line = process.stdout.readline()
        m = line.decode("utf-8")
        debug_output.append(line.decode("utf-8"))
        m = m.rstrip('\r\n')
        if m[-2] == '0':
            break
    s = ''.join(debug_output)
    my_text3.configure(state="normal")
    my_text3.delete(1.0, 'end-1c')
    my_text3.insert(tk.INSERT, os.getcwd() + "\n" + s)
    my_text3.configure(state="disabled")


def stop_debug(a):
    global process
    global last_breakpoint
    process.stdin.write('quit\n'.encode("utf-8"))
    process.stdin.flush()
    process.stdin.write('y\n'.encode("utf-8"))
    process.stdin.flush()
    my_text.tag_remove("breakpoint", str(last_breakpoint) + ".0", str(last_breakpoint) + ".999")
    my_text3.configure(state="normal")
    my_text3.delete(1.0, 'end-1c')
    my_text3.insert(tk.INSERT, os.getcwd())
    my_text3.configure(state="disabled")
    sketchMenu.entryconfigure(6, state='disabled')
    sketchMenu.entryconfigure(7, state='disabled')
    sketchMenu.entryconfigure(8, state='disabled')


def get_line_number():
    output = ''
    row, col = my_text.index("end").split('.')
    for i in range(1, int(row)):
        output += str(i) + '\n'
    print(output)
    return output


def redraw():
    global breakpoint_list
    breakpoint_list_set = set(breakpoint_list)
    my_canvas.delete("all")
    i = my_text.index("@0,0")
    while True:
        dline = my_text.dlineinfo(i)
        if dline is None:
            break
        y = dline[1]
        linenum = str(i).split(".")[0]
        if linenum in breakpoint_list_set:
            my_canvas.create_text(2, y, anchor="nw", text=linenum, font=("Arial", 16), fill="red")
        else:
            my_canvas.create_text(2, y, anchor="nw", text=linenum, font=("Arial", 16))
        i = my_text.index("%s+1line" % i)


def get_breakpoint(event):
    global breakpoint_list
    bp = int(event.y)//24 + 1
    if len(breakpoint_list) == 0:
        breakpoint_list.append(str(bp))
        sketchMenu.entryconfigure(5, state='normal')
    else:
        breakpoint_list_set = set(breakpoint_list)
        if str(bp) in breakpoint_list_set:
            breakpoint_list.remove(str(bp))
            sketchMenu.entryconfigure(5, state='disabled')
        else:
            breakpoint_list.append(str(bp))
            sketchMenu.entryconfigure(5, state='normal')
    redraw()
    print("Clicked at: ", bp)


def get_keyboard_pressed(event):
    key_pressed = repr(event.char)
    if key_pressed == "'{'":
        my_text.insert(my_text.index(tk.INSERT), "}")
    print(key_pressed)


def highlight_text():
    my_text.mark_set("range_start", "1.0")
    data = my_text.get("1.0", "end-1c")
    for token, content in lex(data, CLexer()):
        my_text.mark_set("range_end", "range_start + %dc" % len(content))
        my_text.tag_add(str(token), "range_start", "range_end")
        my_text.mark_set("range_start", "range_end")
    for token, content in lex(data, CppLexer()):
        my_text.mark_set("range_end", "range_start + %dc" % len(content))
        my_text.tag_add(str(token), "range_start", "range_end")
        my_text.mark_set("range_start", "range_end")

    my_text.tag_configure("Token.Keyword", foreground="#CC7A00")
    my_text.tag_configure("Token.Keyword.Constant", foreground="#CC7A00")
    my_text.tag_configure("Token.Keyword.Declaration", foreground="#CC7A00")
    my_text.tag_configure("Token.Keyword.Namespace", foreground="#CC7A00")
    my_text.tag_configure("Token.Keyword.Pseudo", foreground="#CC7A00")
    my_text.tag_configure("Token.Keyword.Reserved", foreground="#CC7A00")
    my_text.tag_configure("Token.Keyword.Type", foreground="#CC7A00")
    my_text.tag_configure("Token.Name", foreground="#003D99")
    my_text.tag_configure("Token.Name.Attribute", foreground="#003D99")
    my_text.tag_configure("Token.Name.Builtin", foreground="#003D99")
    my_text.tag_configure("Token.Name.Builtin.Pseudo", foreground="#003D99")
    my_text.tag_configure("Token.Name.Class", foreground="#003D99")
    my_text.tag_configure("Token.Name.Constant", foreground="#003D99")
    my_text.tag_configure("Token.Name.Exception", foreground="#003D99")
    my_text.tag_configure("Token.Name.Decorator", foreground="#003D99")
    my_text.tag_configure("Token.Name.Entity", foreground="#003D99")
    my_text.tag_configure("Token.Name.Label", foreground="#003D99")
    my_text.tag_configure("Token.Name.Namespace", foreground="#003D99")
    my_text.tag_configure("Token.Name.Other", foreground="#003D99")
    my_text.tag_configure("Token.Name.Tag", foreground="#003D99")
    my_text.tag_configure("Token.Name.Variable", foreground="#003D99")
    my_text.tag_configure("Token.Name.Function", foreground="#003D99")
    my_text.tag_configure("Token.Operator.Word", foreground="#CC7A00")
    my_text.tag_configure("Token.Comment", foreground="#6C666C")
    my_text.tag_configure("Token.Comment.Single", foreground="#6C666C")
    my_text.tag_configure("Token.Comment.Multiline", foreground="#6C666C")
    my_text.tag_configure("Token.Comment.Preproc", foreground="#003D99")
    my_text.tag_configure("Token.Literal", foreground="#248F24")
    my_text.tag_configure("Token.Literal.String", foreground="#248F24")
    my_text.tag_configure("Token.String", foreground="#248F24")
    my_text.tag_configure("Token.Generic", foreground="#4FFF00")
    my_text.tag_configure("Token.Generic.Heading", foreground="#4FFF00")
    my_text.tag_configure("Token.Generic.Subheading", foreground="#4FFF00")
    my_text.tag_configure("Token.Operator", foreground="#FF0000")
    my_text.tag_configure("Token.Operator.Word", foreground="#FF0000")


root = tk.Tk()
root.title("Digital MCU Simulator IDE")
root.iconbitmap('C:/Users/bunna/Desktop/Code/Python/DMCUS/icon.ico')
root.geometry("1200x750")


my_frame = tk.Frame(root)
my_frame.pack(fill=tk.X, side=tk.TOP)

tab_parent = ttk.Notebook(root)
tab1 = ttk.Frame(tab_parent)
tab2 = ttk.Frame(tab_parent)
tab_parent.add(tab1, text="       Result       ")
tab_parent.add(tab2, text="       Debug       ")
tab_parent.pack(expand=1, fill=tk.X, side=tk.TOP)

my_frame2 = tk.Frame(root)
my_frame2.pack(expand=1, fill=tk.X, side=tk.BOTTOM)
status_bar = tk.Label(my_frame2, text='Ready    ', anchor=tk.E)
status_bar.pack(fill=tk.X, side=tk.RIGHT, ipady=5)

text_scroll = tk.Scrollbar(my_frame)
text_scroll.pack(side=tk.RIGHT, fill=tk.Y)
hor_scroll = tk.Scrollbar(my_frame, orient="horizontal")
hor_scroll.pack(side=tk.BOTTOM, fill=tk.X)

my_text = tk.Text(my_frame, width=108, height=22, font=("Arial", 16), selectbackground="yellow", selectforeground="black", undo=True, yscrollcommand=text_scroll.set, xscrollcommand=hor_scroll.set, wrap="none")
my_text2 = tk.Text(tab1, width=95, height=5, font=("Arial", 16))
my_text2.insert(tk.INSERT, os.getcwd() + "\n")
my_text2.configure(state='disabled')
my_text2.pack(fill=tk.X)
my_text3 = tk.Text(tab2, width=100, height=5, font=("Arial", 16))
my_text3.pack(fill=tk.X)
my_text3.insert(tk.INSERT, os.getcwd() + "\n")
my_text3.configure(state='disabled')

my_canvas = tk.Canvas(my_frame, width=25, height=22)
my_canvas.pack(fill=tk.BOTH, side=tk.LEFT, padx=5, pady=5)

Pmw.initialise(root)
balloon = Pmw.Balloon(root)
balloon.bind(my_canvas, "Click on the number to set breakpoints")

my_text.pack(fill=tk.BOTH, side=tk.RIGHT, padx=5, pady=5)
root.bind("<Return>", lambda event: redraw())
root.bind("<BackSpace>", lambda event: redraw())
my_text.bind("<MouseWheel>", lambda event: redraw())
root.bind("<space>", lambda event: redraw())
text_scroll.bind("<Motion>", lambda event: redraw())
my_canvas.bind("<Button-1>", get_breakpoint)
my_text.bind("<BackSpace>", lambda event: highlight_text())
my_text.bind("<space>", lambda event: highlight_text())
my_text.bind("<Return>", lambda event: highlight_text())
my_text.bind("<Key>", get_keyboard_pressed)
#root.bind("<space>", lambda event: highlight_text())
#root.bind("<Return>", lambda event: highlight_text())

text_scroll.config(command=my_text.yview)
hor_scroll.config(command=my_text.xview)

menubar = tk.Menu(root)
root.config(menu=menubar)

fileMenu = tk.Menu(menubar, tearoff=False)
fileMenu.add_command(label="New", command=lambda: new_file(False), accelerator="Ctrl+N")
fileMenu.add_command(label="Open", command=lambda: open_file(False), accelerator="Ctrl+O")
fileMenu.add_command(label="Save", command=lambda: save_file(False), accelerator="Ctrl+S")
fileMenu.add_command(label="Save As", command=lambda: save_as(False), accelerator="Ctrl+S")
fileMenu.add_separator()
fileMenu.add_command(label="Exit", command=exit_app)
menubar.add_cascade(label="File", menu=fileMenu)

editMenu = tk.Menu(menubar, tearoff=False)
editMenu.add_command(label="Cut", command=lambda: cut_text(False), accelerator="Ctrl+X")
editMenu.add_command(label="Copy", command=lambda: copy_text(False), accelerator="Ctrl+C")
editMenu.add_command(label="Paste", command=lambda: paste_text(False), accelerator="Ctrl+V")
editMenu.add_separator()
editMenu.add_command(label="Find", command=lambda: find_text(False), accelerator="Ctrl+F")
editMenu.add_separator()
editMenu.add_command(label="Undo", command=my_text.edit_undo, accelerator="Ctrl+Z")
editMenu.add_command(label="Redo", command=my_text.edit_redo, accelerator="Ctrl+Y")
menubar.add_cascade(label="Edit", menu=editMenu)

sketchMenu = tk.Menu(menubar, tearoff=False)
sketchMenu.add_command(label="Compile", command=lambda: compile(False), accelerator="F5")
sketchMenu.add_command(label="Stop", command=lambda: stop(process))
sketchMenu.add_separator()
sketchMenu.add_command(label="Upload", command=lambda: upload(False))
sketchMenu.add_separator()
sketchMenu.add_command(label="Start Debug", command=lambda: start_debug(False))
sketchMenu.add_command(label="Step", command=lambda: step_debug(False))
sketchMenu.add_command(label="Continue", command=lambda: continue_debug(False))
sketchMenu.add_command(label="Stop Debug", command=lambda: stop_debug(False))
menubar.add_cascade(label="Sketch", menu=sketchMenu)
sketchMenu.entryconfigure(5, state='disabled')
sketchMenu.entryconfigure(6, state='disabled')
sketchMenu.entryconfigure(7, state='disabled')
sketchMenu.entryconfigure(8, state='disabled')

helpMenu = tk.Menu(menubar, tearoff=False)
helpMenu.add_command(label="Help", command=lambda: webbrowser.open("https://tamu.edu"))
helpMenu.add_separator()
helpMenu.add_command(label="About Us", command=about_us)
menubar.add_cascade(label="Help", menu=helpMenu)

root.bind('<Control-Key-x>', cut_text)
root.bind('<Control-Key-c>', copy_text)
root.bind('<Control-Key-v>', paste_text)
root.bind('<Control-Key-n>', new_file)
root.bind('<Control-Key-o>', open_file)
root.bind('<Control-Key-s>', save_file)
root.bind('<Control-Key-f>', find_text)
root.bind('<F5>', compile)
root.bind('<Control-Key-p>', get_line_number())

root.protocol("WM_DELETE_WINDOW", exit_app)
root.mainloop()
