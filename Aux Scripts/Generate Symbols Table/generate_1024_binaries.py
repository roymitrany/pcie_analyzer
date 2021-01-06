file = open(r"./1024_binaries_input","w+") 


for num in range(0,1024):
        binary = format(num, "010b")
        binary = '0b' + str(binary)
        #print (binary)
        
        file.write(binary + "\n")