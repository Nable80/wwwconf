bool IsRobotNick(char *nick)  // code by ring0 & jetsys
{
	static char robot_rus[]="робот-мурзолог-";
	static char robot_lat[]="poбoт-мypзoлoг-";
	if(!nick) 
		return false;
	for(int i = 0; i <= 14; i++) {
		if(
			nick[i] != robot_rus[i] && 
			nick[i] != robot_lat[i]
			) 
			return false;
	}
	return true;
}
