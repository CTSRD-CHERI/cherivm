import java.nio.ByteBuffer;
import java.io.UnsupportedEncodingException;

public class SodiumTest {

	private static native void initIDs();
	private static native int getPublicKeySize();
	private static native int getPrivateKeySize();
	private static native int getNonceSize();
	private static native void generateKeyPair(ByteBuffer bufPublic, ByteBuffer bufPrivate);
	private static native void generateNonce(ByteBuffer bufNonce);

	private static native int encryptLength(int length);
	private static native void encryptData(ByteBuffer bbufData, ByteBuffer bbufSenderPrivate, ByteBuffer bbufRecipientPublic, ByteBuffer bbufNonce, ByteBuffer bbufOutput);
	private static native int decryptLength(int length);
	private static native void decryptData(ByteBuffer bbufData, ByteBuffer bbufRecipientPrivate, ByteBuffer bbufSenderPublic, ByteBuffer bbufNonce, ByteBuffer bbufOutput);

	private static final int SIZE_PUBLICKEY;
	private static final int SIZE_PRIVATEKEY;
	private static final int SIZE_NONCE;

	public static boolean direct;
	
	static {
		System.out.print("[opening libjsodium... ");
		System.loadLibrary("jsodium");
		initIDs();
		System.out.println("OK]");
		
		SIZE_PUBLICKEY = getPublicKeySize();
		SIZE_PRIVATEKEY = getPrivateKeySize();
		SIZE_NONCE = getNonceSize();
	}

	private static String toHex(ByteBuffer buf) {
		StringBuilder sb = new StringBuilder();
		for (byte b : buf.array()) {
			sb.append(Character.forDigit((b >> 4) & 0xF, 16)); 
			sb.append(Character.forDigit((b & 0xF), 16)); 
		}
		return sb.toString();
	}
	
	public static class SodiumUser {
		private final String name;
		private final ByteBuffer keyPublic;
		private final ByteBuffer keyPrivate;
		
		public SodiumUser(String name) {
			System.out.print("[create user " + name + "... ");
			
			this.name = name;
			this.keyPublic = direct ? ByteBuffer.allocateDirect(SIZE_PUBLICKEY) : ByteBuffer.allocate(SIZE_PUBLICKEY);
			this.keyPrivate = direct ? ByteBuffer.allocateDirect(SIZE_PRIVATEKEY) : ByteBuffer.allocate(SIZE_PRIVATEKEY);

			generateKeyPair(this.keyPublic, this.keyPrivate);

			System.out.println("DONE]");
			// System.out.println("[key pair: " + toHex(this.keyPublic) + ", " + toHex(this.keyPrivate) + "]");
		}

		public SodiumMessage encrypt(String msg, SodiumUser recipient) throws UnsupportedEncodingException {
			long startTime, totalTime;
			byte[] data = msg.getBytes("UTF-8");

			System.out.print("[generating a nonce... ");
			ByteBuffer bufNonce = direct ? ByteBuffer.allocateDirect(SIZE_NONCE) : ByteBuffer.allocate(SIZE_NONCE);
			generateNonce(bufNonce);
			System.out.println("DONE]");

			System.out.print("[encrypting message... ");
			int encryptedLength = encryptLength(data.length);
			ByteBuffer bufOutput = direct ? ByteBuffer.allocateDirect(encryptedLength) : ByteBuffer.allocate(encryptedLength);
			encryptData(ByteBuffer.wrap(data), this.keyPrivate, recipient.keyPublic, bufNonce, bufOutput);
			System.out.println("DONE]");

			return new SodiumMessage(bufOutput, bufNonce);
		}

		public String decrypt(SodiumMessage msg, SodiumUser sender) throws UnsupportedEncodingException {
			System.out.print("[decrypting message... ");
			int decryptedLength = decryptLength(msg.getData().limit());
			ByteBuffer bufOutput = ByteBuffer.allocate(decryptedLength);
			decryptData(msg.getData(), this.keyPrivate, sender.keyPublic, msg.getNonce(), bufOutput);
			System.out.println("DONE]");

			return new String(bufOutput.array(), "UTF-8");
		}
	}

	public static class SodiumMessage {
		private final ByteBuffer bufData, bufNonce;

		public SodiumMessage(ByteBuffer bufData, ByteBuffer bufNonce) {
			this.bufData = bufData;
			this.bufNonce = bufNonce;
		}

		public ByteBuffer getData() {
			return this.bufData;
		}

		public ByteBuffer getNonce() {
			return this.bufNonce;
		}
	}
	
	public static void main(String[] args) throws UnsupportedEncodingException {
		direct = (args.length > 0);

		System.out.println("*** LIBSODIUM JNI TEST " + (direct ? "(direct)" : "") + " ***");
		
		SodiumUser userAlice = new SodiumUser("Alice");
		SodiumUser userBob = new SodiumUser("Bob");
		SodiumUser userEve = new SodiumUser("Eve");

		String msgBob = "Meet me in the pub in 10mins";

		SodiumMessage msg = userBob.encrypt(msgBob, userAlice);
		String msgAlice = userAlice.decrypt(msg, userBob);

		System.out.println("Bob sent:        " + msgBob);
		System.out.println("Alice decrypted: " + msgAlice);
	}
}
