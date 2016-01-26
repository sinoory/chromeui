#build sh

# release mode in default
BUILD_DEBUG=0

# Read command parameters.
for arg in "$@"
do
  if [ $arg -a $arg = "Debug" ];then
    BUILD_DEBUG=1
  fi
done

if [ $BUILD_DEBUG == 1 ] ; then
   echo "........BUILDING DEBUG MODE......"
   ninja -C out/Debug views_examples_exe
else
   echo "........BUILDING RELEASE MODE......"
   ninja -C out/Release views_examples_exe
fi


