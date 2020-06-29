for i in {1..10};do
        dmesg --clear
        ./user_program/master 1 ./input/target_file_${i} f & ./user_program/slave 1 ./output/fnctl_result_${i} f 127.0.0.1
        dmesg | grep master
        sleep 1s
        diff ./input/target_file_${i} ./output/fnctl_result_${i}
        sleep 1s
done

dmesg --clear
./user_program/master 1 ./input/target_file f & ./user_program/slave 1 ./output/fnctl_result f 127.0.0.1
dmesg | grep master
sleep 1s
diff ./input/target_file ./output/fnctl_result
