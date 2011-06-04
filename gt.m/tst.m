tst;
;
init
 ;S $ZGBLDIR="/home/ivan/tmp/mginx-test/mumps.gld"
 Q
;
setData(data)
 ;if ($G(cur,"")'=data) S $ZGBLDIR="mumps"_data_".gld" ZGDCLNUP  S cur=data,tmp=$INCREMENT(^LOAD)
 ;if ($ZGBLDIR'="mumps"_data_".gld") S $ZGBLDIR="mumps"_data_".gld" ZGDCLNUP  
 TSTART
 S tmp=$INCREMENT(^DATA(data))
 S tmp=$INCREMENT(@("^DATA"_data_"("_data_")"))
 TCOMMIT
 Q
