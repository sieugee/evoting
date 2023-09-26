echo "Rebuild programs."
bash build.sh
echo "Prepare for the vote."
out/vote_prepare.o
echo "Start vote demo, random votes"
out/demo_vote.o
echo "Start counting vote for the last vote"
out/demo_count_vote.o