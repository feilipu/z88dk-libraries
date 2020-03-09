Creates all the demo application Tasks, then starts the Scheduler.

`main.c` creates a task called "Print".  This only executes every two seconds but has the highest priority so is guaranteed to get processor time.

Its main function is to check that all the other tasks are still operational. Nearly all the tasks in the demo application maintain a unique count that is incremented each time the task successfully completes its function.  Should any error occur within the task the count is permanently halted.  The print task checks the count of each task to ensure it has changed since the last time the print task executed.

If all the tasks are still incrementing their unique counts, the Print task displays an "OK" message.

The Print task blocks on the queue into which messages that require displaying are posted.  It will therefore only block for the full 2 seconds if no messages are posted onto the queue.

```sh
zcc +yaz180 -subtype=app -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node200000 --math32 -llib/yaz180/freertos @common.lst -o common -create-app
cat > /dev/ttyUSB0 < common.ihx
```

```sh
zcc +scz180 -subtype=hbios -clib=sdcc_iy -SO3 -v -m --list --max-allocs-per-node200000 --math32 -llib/scz180/freertos @common.lst -o common -create-app
cat > /dev/ttyUSB0 < common.ihx
```
