```
plink.exe -serial COM6 -sercfg 3000000
```
```
hexdump -C build/img/predOS.img | head
```


```
		ud.block.buf[ud.received_bytes] = c;
        ud.received_bytes++;
        if(ud.received_bytes == EMMC_BLOCK_SIZE){
          ud.received_bytes = 0;
          ud.received_blocks++;

          if(ud.received_blocks >= UART_BLOCK_BUFFER_SIZE){
            ud.recieved_blocks = 0;
            for(uint32_t i; i < UART_BLOCK_BUFFER_SIZE; ++i){
              emmc_write_block(i + ud.total_received_blocks, &ud.block[i])
            }
            ud.total_received_blocks += UART_BLOCK_BUFFER_SIZE;
            uart_print("\n", 1);
          }
          if(ud.received_blocks >= ud.image_size){
            ud.state = STANDBY;
            SYS_LOG("received image size of %d", ud.received_blocks);
            
          }
        }
        break;
```