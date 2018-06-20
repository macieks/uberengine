<?php

require 'nsContentUtils.php';

$query = beginQuery();
if (is_null($query))
	return;
	
if ($query == "best_timers")
{
	$start_rank = getIntMin("start_rank", 0);
	if (is_null($start_rank))
		return;
	
	$max_count = getIntMinMax("max_count", 1, 20);
	if (is_null($max_count))
		return;

	$query_string = "SELECT @rank_index := @rank_index + 1, bestTime, userName FROM (SELECT @rank_index := $start_rank) t, test_leaderboards ORDER BY bestTime ASC, dateSubmitted ASC LIMIT $start_rank, $max_count";
	endQuery($query_string);
}

?>
