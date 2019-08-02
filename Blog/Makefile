.PHONY:ALL
all:insert select

insert:insert.cc
	gcc $^ -o $@ -L/usr/lib64/mysql -lmysqlclient

select:select.cc
	gcc $^ -o $@ -L/usr/lib64/mysql -lmysqlclient

.PHONY:clean
clean:
	rm main
