import java.util.*;
class Solution {
    public string smallestSubsequence(string s){
        int[] freq=new int[26];
        boolean[] seen=new boolean[26];
        Stack <Character> stack = new Stack<>();
        for (int i=0;i<s.length();i++){
            freq[s.charAt(i)-'a']++;
        }
        for (char c:s.toCharArray()){
            freq[c-'a']--;
            if (seen[c-'a']){
                continue;
            }
            while(!stack.isEmpty()&&stack.peek()>c&&freq[stack.peek()-'a']>0){
                seen[stack.pop()-'a']=false;
            }
            stack.push(c);
            seen[c-'a']=true;
        }
        StringBuilder ans=new StringBuilder();
        for (char c:stack){
            ans.append(c);
        }
        return ans.toString();
    }
}

