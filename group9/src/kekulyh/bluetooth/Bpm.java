package kekulyh.bluetooth;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class Bpm extends Activity {
	private CircleBar circleBar;
	//private EditText setnum;
	private Button setnumbutton;
	int i = 0;
	
	private OutputStream os;
	byte[] bytes = "1".getBytes();
	
	private InputStream is;
	byte[] readbytes = new byte[30];
	
	private int bpm = 0;

	private TextView alarm;
	

//    public static int byte2int(byte[] res) {   
//    // 一个byte数据左移24位变成0x??000000，再右移8位变成0x00??0000   
//      
//    int targets = (res[0] & 0xff) | ((res[1] << 8) & 0xff00) // | 表示按位或   
//    | ((res[2] << 24) >>> 8) | (res[3] << 24);   
//    return targets;   
//    }   
//    
//	 public static int byteArrayToInt(byte[] b, int offset) {
//	       int value= 0;
//	       for (int i = 0; i < 3; i++) {
//	    	   value|=b[i];
//	    	   value=value<<8;
//	       }
//	       return value;
//	 }
//
//	
//	
//	public static byte[] toByteArray(InputStream input) throws IOException {
//		 
//		  ByteArrayOutputStream swapStream = new ByteArrayOutputStream();
//		        byte[] buff = new byte[1024];
//		        int rc = 0;
//		        while ((rc = input.read(buff, 0, 1024)) > 0) {
//		            swapStream.write(buff, 0, rc);
//		        }
//		        byte[] bytes = swapStream.toByteArray();
//		        return bytes; 
//	}
	
	
//    /** 
//     * 读取流 
//     *  
//     * @param inStream 
//     * @return 字节数组 
//     * @throws Exception 
//     */  
//    public static byte[] readStream(InputStream inStream) throws Exception {  
//        ByteArrayOutputStream outSteam = new ByteArrayOutputStream();  
//        byte[] buffer = new byte[1024];  
//        int len = -1;  
//        while ((len = inStream.read(buffer)) != -1) {  
//            outSteam.write(buffer, 0, len);  
//        }  
//        outSteam.close();  
//        inStream.close();  
//        return outSteam.toByteArray();  
//    }  
	
	
    
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.bpm);
		circleBar = (CircleBar) findViewById(R.id.circle);
		//setnum = (EditText) findViewById(R.id.setnum);
		setnumbutton = (Button) findViewById(R.id.setnumbutton);
		circleBar.setMaxstepnumber(200);
		
		alarm = (TextView)findViewById(R.id.alarm);
		alarm.setText(null);
		
		setnumbutton.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				
				
		        Thread thread=new Thread(new Runnable()  
		        {  
		            @Override  
		            public void run()  
		            {  
		                Log.e("thread", "begin");  
		                // TODO Auto-generated method stub  
						
		                
		                
		                
		                //发送1给蓝牙
						try {
							os = BlueTooth.btSocket.getOutputStream();
							os.write(bytes);
							os.flush();
							//os.close();
							
							String strsend = new String(bytes);
							int step1 = Integer.valueOf(strsend);
							
							System.out.println("send byte[] is " + (bytes[0]-48));
							System.out.println("string sent is " + strsend);
							System.out.println("int of string sent is " + step1);
							System.out.println(os);
							System.out.println("outputstream test");
							
							Log.e("outputstream", "test");
						} catch (IOException e1) {
							// TODO Auto-generated catch block
							e1.printStackTrace();
						}
						
						try {
							if (BlueTooth.btSocket.getInputStream() != null) {
							
								is = BlueTooth.btSocket.getInputStream();
							
								int step = 0;
								int number = is.read(readbytes);
								String strread = new String(readbytes);
//								step = Integer.valueOf(strread);
//								if(bpm < 250 & bpm > 0){
//						        	bpm = step;
//						        }
							
								if(number < 4){
									int value = 0;
								 
									for (int i = 0; i < number; i++) {
										int shift = (number - 1 - i) ;
										value += ((readbytes[i]-48) * Math.pow(10, shift));
								    }
							        step = value;
							        //if(step < 250 && step > 0){
							        	bpm = step;
							        //}
							        	
								}
							    
								System.out.println(is);
								System.out.println("read byte number is " + number);
								for(int j = 0; j < readbytes.length; j++){ System.out.println(readbytes[j]); }
								System.out.println("string read is " + strread);
								System.out.println("readbytes int is " + step);
								System.out.println("inputstream test");
							
								
							
								Log.e("inputstream","test");
							}
						} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
						}
		                
		                
		                
		                
		                
		            }  
		        });  
		        thread.start();  
		        
		        
		        circleBar.update(bpm, 500);
		        
		        if(bpm<30){
		        	alarm.setText("Low Heart Rate !");
	        	}
		        else{
		        	alarm.setText(null);
		        }
		        
				
				
			}
				
			
		});

	}

}
