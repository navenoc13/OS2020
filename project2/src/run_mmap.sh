for i in {1..10};do
        dmesg --clear
        ./user_program/master 1 ./input/target_file_${i} m & ./user_program/slave 1 ./output/mmap_result_${i} m 127.0.0.1
        dmesg | grep master
        sleep 1s
        dmesg | grep slave
        sleep 1s
        diff ./input/target_file_${i} ./output/mmap_result_${i}
        sleep 1s
done

dmesg --clear
./user_program/master 1 ./input/target_file m & ./user_program/slave 1 ./output/mmap_result m 127.0.0.1
dmesg | grep master
sleep 1s
dmesg | grep slave
sleep 1s
diff ./input/target_file ./output/mmap_result
