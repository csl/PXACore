;;------------------------------------------------------------------------
;;  Author               : Garry
;;  Original Date        : May,15,2004
;;  FileName             : fmtldrf.asm
;;  Function             :
;;                         This module countains the load flopy creating code.
;;                         When a flopy is formated newly,it must be process-
;;                         ed by this program in order to load the Hello Taiwan
;;                         operating system.
;;  Last modified date   :
;;  Last modified author :
;;  Modified content     :
;;------------------------------------------------------------------------

    bits 16                      ;;This program is running under real address
                                 ;;mode.
    org 0x0100                   ;;This is a .com format DOS executable module.

    jmp gl_start                 ;;Jump to the code section.

gl_datasection:                  ;;The following is data section.

    strBegin
             db 0x0d
             db 0x0a
             db 'Creating load disk now,may be take several minutes.'
             db 0x0d
             db 0x0a
             db 'Waiting please.'
             db 0x0d
             db 0x0a
             db 0x00             ;;The end flag of the string.

    strProcess
             db '.'

    strError                     ;;If error occured,the following message will
                                 ;;be print out.
             db 0x0d
             db 0x0a
             db 'Some error occured.'
             db 0x0d
             db 0x0a
             db 'Please make sure your flopy is formatted newly and'
             db ' correct,and the current directory countains the following'
             db ' files:'
             db 0x0a
             db 0x0d
             db 'master.bin'
             db 0x0a
             db 0x0d
             db 'miniker.bin'
             db 0x0a
             db 0x0d
             db 'realinit.bin'
             db 0x0a
             db 0x0d
             db 'bootsect.bin'
             db 0x0a
             db 0x0d
             db 0x00

    strOK    db 0x0d             ;;When create the load disk OK,print out it.
             db 0x0a
             db 'Create load disk succefully.'
             db 0x0a
             db 0x0d
             db 0x00

    strMastName
             db 'master.bin'
             db 0x00
    strMiniName
             db 'miniker.bin'
             db 0x00
    strRealName
             db 'realinit.bin'
             db 0x00
    strBootName
             db 'bootsect.bin'
             db 0x00

    Buffer                       ;;Program buffer,this program load the file
                                 ;;content into this buffer,and then write
                                 ;;it to the floppy.
                                 ;;Here reserved 8K buffer.
     times 1024 dd 0x00000000
     times 1024 dd 0x00000000

gl_start:                        ;;The code section.Program starts here.

    mov ax,strBegin              ;;First,print out the beginning msg.
    call np_strlen
    mov ax,strBegin
    call np_printmsg

    call np_writemast            ;;Write the master to floppy.
    cmp ax,0x00
    je .ll_error

    call np_writemini            ;;Write the mini kernal into floppy.
    cmp ax,0x00
    je .ll_error

    call np_writereal            ;;Write the real mode initialize code.
    cmp ax,0x00
    je .ll_error

    call np_writeboot            ;;Write the load sector.
    cmp ax,0x00
    je .ll_error

    mov ax,strOK
    call np_strlen
    mov ax,strOK
    call np_printmsg
    jmp .ll_exit

.ll_error:
    mov ax,strError
    call np_strlen
    mov ax,strError
    call np_printmsg
.ll_exit:
    mov ax,0x4c00                ;;End of the program,return to DOS.
    int 0x21

;;------------------------------------------------------------------------
;;  The following are some helper functions.
;;------------------------------------------------------------------------

np_writemast:                    ;;Read the master and write it.

    mov dx,strMastName
    mov al,0x00
    mov ah,0x3d
    int 0x21                     ;;Open it.
    jc .ll_error
    mov word [.nr_filenumber],ax  ;;Save the file handle.

.ll_begin:
    mov bx,word [.nr_filenumber]
    mov dx,Buffer
    mov cx,0x0400
    mov ah,0x03f
    int 0x21                     ;;Read 1k data from file.
    jc .ll_error

    cmp ax,0x0000                ;;If the file is read over.
    jz .ll_end

    mov al,0x02                  ;;Write 2 sectors once time.
    mov ah,0x03
    mov ch,byte [.nr_track]
    mov cl,byte [.nr_sector]
    mov dx,0x0000
    mov bx,Buffer
    int 0x13                     ;;Write it to flopy.
    jc .ll_error
    add byte [.nr_sector],0x02
    cmp byte [.nr_sector],18      ;;If write one track.
    jae .ll_inc_track
    jmp .ll_continue
.ll_inc_track:
    call np_printprocess

    inc byte [.nr_track]
    mov byte [.nr_sector],0x01
.ll_continue:
    sub word [.nr_totals],0x02
    jnz .ll_begin
    jmp .ll_end
.ll_error:
    mov ax,0x0000
    ret
    .nr_sector     db 0x0d        ;;Local variables,to be used by this proc.
    ;.nr_sector     db 0x0f
    .nr_track      db 0x07
    ;.nr_track      db 0x06
    .nr_totals     dw 1120        ;;The master.bin occupied 560k.
    .nr_filenumber dw 0x00
.ll_end:
    mov ax,0xffff
    ret                          ;;End of the procedure.

np_writemini:                    ;;Read the mini kernal and write it.

    mov dx,strMiniName
    mov al,0x00
    mov ah,0x3d
    int 0x21                     ;;Open it.
    jc .ll_error
    mov word [.nr_filenumber],ax  ;;Save the file handle.

.ll_begin:
    mov bx,word [.nr_filenumber]
    mov dx,Buffer
    mov cx,0x0400
    mov ah,0x03f
    int 0x21                     ;;Read 1k data from file.
    jc .ll_error

    cmp ax,0x0000
    jz .ll_end

    mov al,0x02                  ;;Write 2 sectors once time.
    mov ah,0x03
    mov ch,byte [.nr_track]
    mov cl,byte [.nr_sector]
    mov dx,0x0000
    mov bx,Buffer
    int 0x13                     ;;Write it to flopy.
    jc .ll_error
    add byte [.nr_sector],0x02
    cmp byte [.nr_sector],18      ;;If write one track.
    jae .ll_inc_track
    jmp .ll_continue
.ll_inc_track:
    call np_printprocess

    inc byte [.nr_track]
    mov byte [.nr_sector],0x01
.ll_continue:
    sub byte [.nr_totals],0x02
    jnz .ll_begin
    jmp .ll_end
.ll_error:
    mov ax,0x0000
    ret
    .nr_sector     db 0x0b        ;;Local variables,to be used by this proc.
    .nr_track      db 0x00
    .nr_totals     dw 128
    ;.nr_totals     dw 112         ;;--------- ** debug ** ----------------
    .nr_filenumber dw 0x00
.ll_end:
    mov ax,0xffff
    ret                          ;;End of the procedure.

np_writereal:                    ;;This procedure reads the realinit.bin and
                                 ;;write it to disk.
    mov dx,strRealName
    mov al,0x00
    mov ah,0x3d
    int 0x21                     ;;Open file to read.
    jc .ll_error
    mov bx,ax
    mov dx,Buffer
    mov cx,0x01000
    mov ah,0x03f
    int 0x21                     ;;Read 4k data from file,because the real-
                                 ;;init.bin's size is 4k,so we only read once.
    jc .ll_error

    mov al,0x08
    mov ah,0x03
    mov ch,0x00
    mov cl,0x03
    mov dx,0x0000
    mov bx,Buffer
    int 0x13                     ;;Write it.
    jc .ll_error
    jmp .ll_end
.ll_error:
    mov ax,0x00
    ret
.ll_end:
    mov ax,0xffff
    ret                          ;;End of the procedure.

np_writeboot:                    ;;Write the bootsect.bin to floppy.

    mov dx,strBootName
    mov al,0x00
    mov ah,0x3d
    int 0x21                     ;;Open file to read.
    jc .ll_error
    mov bx,ax
    mov dx,Buffer
    mov cx,0x0200
    mov ah,0x03f
    int 0x21                     ;;Read 512B data from file,because the boot-
                                 ;;sect.
    jc .ll_error

    mov al,0x01
    mov ah,0x03
    mov ch,0x00
    mov cl,0x01
    mov dx,0x0000
    mov bx,Buffer
    int 0x13                     ;;Write it.
    jc .ll_error
    jmp .ll_end
.ll_error:
    mov ax,0x00
    ret
.ll_end:
    mov ax,0xffff

    ret                          ;;End of the procedure.

np_printmsg:                     ;;This procedure print out some message,
                                 ;;the ax register indicates the base address
                                 ;;of the string,and cx is the length of the
                                 ;;string.
                                 ;;CAUTION: After return,the ax register's
                                 ;;value may be changed.

    push bp                      ;;Save the registers used by this proc.
    push bx
    push ax                      ;;Some registers not used in this procedu-
                                 ;;re maybe used in the int 0x010,so we must
                                 ;;save them too.Fuck BIOS!!!!!
    push cx
    mov ah,0x03
    mov bh,0x00
    int 0x010                    ;;Read the position of cursor.
    mov bx,0x0007
    mov ax,0x1301
    pop cx
    pop bp
    int 0x010                    ;;Print out the the message.
    pop bx
    pop bp
    ret                          ;;End of the procedure.

np_strlen:                       ;;This procedure get a string's length.
                                 ;;A zero indicates the string's end,the
                                 ;;same as C string.
                                 ;;The ax countains the base address of
                                 ;;the string,and after return,the cx coun-
                                 ;;tains the result.
                                 ;;CAUTION: After return,the ax's value may
                                 ;;be changed.
    push bp
    push bx
    mov bx,ax
    mov al,0x00
    xor cx,cx
.ll_begin:
    cmp byte [bx],al
    je .ll_end
    inc bx
    inc cx
    jmp .ll_begin
.ll_end:
    pop bx
    pop bp
    ret                          ;;End of the string.

np_printprocess:                 ;;Print out some dot,indicates the program
                                 ;;is running OK.
    push ax
    push bx
    push cx
    push dx
    push bp
    mov ah,0x03
    mov bh,0x00
    int 0x10
    mov bx,0x0007
    mov ax,0x1301
    mov bp,strProcess
    mov cx,0x01
    int 0x10
    pop bp
    pop dx
    pop cx
    pop bx
    pop ax
    ret                          ;;End of the procedure.
