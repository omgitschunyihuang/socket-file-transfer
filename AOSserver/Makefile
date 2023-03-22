object = main
object2 = main2
c_file = server.c 
c_file2 = client.c

$(object):$(c_file)
	gcc -o $(object) $(c_file) -lpthread

$(object2):$(c_file2)
	gcc -o $(object2) $(c_file2)

clean:
	rm -f $(object) $(object2)

