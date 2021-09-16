echo COMPILANDO...
gcc -g -o2 --std=c99 -Wall -o sum sum.c
gcc -g -o2 --std=c99 -Wall -o par_sum par_sum.c -lpthread

echo EXECUTANDO SEQUENCIAL SUM...
echo exemplo 5...
time ./sum exemplo5.txt
echo ==============
echo exemplo 6...
time ./sum exemplo6.txt 
echo ==============
echo exemplo 7...
time ./sum exemplo7.txt
echo ==============


echo EXECUTANDO PARALELO PAR_SUM

echo EXEMPLO 5 
echo 2 threads
time ./par_sum -f exemplo5.txt -t 2
echo 4 threads
time ./par_sum -f exemplo5.txt -t 4
echo ===============
echo EXEMPLO 6
echo 2 threads
time ./par_sum -f exemplo6.txt -t 2
echo 4 threads
time ./par_sum -f exemplo6.txt -t 4
echo ===============
echo EXEMPLO 7
echo 2 threads
time ./par_sum -f exemplo7.txt -t 2
echo 4 threads
time ./par_sum -f exemplo7.txt -t 4