if [ "$1" = "clean" ]; then
    rm -f cscope.files cscope.out
    exit 0
fi

find . -type f | grep -v git | grep -v ~$ | grep -v cscope > cscope.files
cscope -buvk
