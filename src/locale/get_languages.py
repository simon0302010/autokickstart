# system-config-language has to be installed for this to work
print("struct Lang locales[] = {")
with open("/usr/share/system-config-language/locale-list", "r") as f:
    for line in f:
        split_line = line.strip().split(" ")
        print("    { \"" + ' '.join(split_line[3:]) + "\", \"" + split_line[0] + "\" },")
print("    { NULL, NULL }")
print("};")