nclient=$1
wcL=$(cat uti$nclient.txt | wc -l)

cat  uti$nclient.txt | awk -v wcL="$wcL" -v load="$nclient" '
	BEGIN{
		sum=0
	}
	    
	{
		sum=sum+$1
	}

	END{
		printf("Average Utilization for (load level = %d) = %f\n", load, 100-(sum/wcL))
	}'