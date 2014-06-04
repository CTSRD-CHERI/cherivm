import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.util.Arrays;
import java.util.Random;
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
	
	public static class SodiumSecurity extends SecurityManager {
		@Override
		public void checkRead(String file) {
			// System.out.println("SodiumSecurity: " + file);
		}
	}
	
	private static float[] generateMatrix(int size) {
		int elems = size * size;
		float[] matrix = new float[elems];
		Random rand = new Random();
		
		for (int i = 0; i < elems; i++)
			matrix[i] = rand.nextFloat();
		
		return matrix;
	}
	
	private static native void squareMatrix(int size, float[] matrixOrig, float[] matrixNew);
	private static native boolean readAccess(String path);
	
	private static void squareMatrix_pure(int size, float[] matrixOrig, float[] matrixNew) {
		int i, j, k;
		for (i = 0; i < size; i++) {
			for (j = 0; j < size; j++) {
				matrixNew[i*size + j] = 0.0f;
				for (k = 0; k < size; k++)
					matrixNew[i*size + j] += matrixOrig[i*size + k] * matrixOrig[k*size + j];
			}
		}
	}
	
	public static class AccessSecurity0 extends SecurityManager {
		@Override
		public void checkRead(String file) {
			// System.out.println("Security0");
			return;
		}
	}

	public static class AccessSecurity1 extends SecurityManager {
		@Override
		public void checkRead(String file) {
			// System.out.println("Security1");
			if (file.startsWith("1/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("1/bin/")) throw new SecurityException();
			if (file.startsWith("1/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("1/proc/sys/acpi/lenovo")) throw new SecurityException();
		}
	}

	public static class AccessSecurity2 extends SecurityManager {
		@Override
		public void checkRead(String file) {
			// System.out.println("Security2");
			if (file.startsWith("1/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("1/bin/")) throw new SecurityException();
			if (file.startsWith("1/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("1/proc/sys/acpi/lenovo")) throw new SecurityException();
			if (file.startsWith("2/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("2/bin/")) throw new SecurityException();
			if (file.startsWith("2/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("2/proc/sys/acpi/lenovo")) throw new SecurityException();
		}
	}
	
	public static class AccessSecurity3 extends SecurityManager {
		@Override
		public void checkRead(String file) {
			// System.out.println("Security3");
			if (file.startsWith("1/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("1/bin/")) throw new SecurityException();
			if (file.startsWith("1/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("1/proc/sys/acpi/lenovo")) throw new SecurityException();
			if (file.startsWith("2/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("2/bin/")) throw new SecurityException();
			if (file.startsWith("2/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("2/proc/sys/acpi/lenovo")) throw new SecurityException();
			if (file.startsWith("3/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("3/bin/")) throw new SecurityException();
			if (file.startsWith("3/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("3/proc/sys/acpi/lenovo")) throw new SecurityException();
		}
	}

	public static class AccessSecurity4 extends SecurityManager {
		@Override
		public void checkRead(String file) {
			// System.out.println("Security4");
			if (file.startsWith("1/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("1/bin/")) throw new SecurityException();
			if (file.startsWith("1/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("1/proc/sys/acpi/lenovo")) throw new SecurityException();
			if (file.startsWith("2/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("2/bin/")) throw new SecurityException();
			if (file.startsWith("2/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("2/proc/sys/acpi/lenovo")) throw new SecurityException();
			if (file.startsWith("3/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("3/bin/")) throw new SecurityException();
			if (file.startsWith("3/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("3/proc/sys/acpi/lenovo")) throw new SecurityException();
			if (file.startsWith("4/usr/local/bin/")) throw new SecurityException();
			if (file.startsWith("4/bin/")) throw new SecurityException();
			if (file.startsWith("4/home/db538/cherivm/target")) throw new SecurityException();
			if (file.startsWith("4/proc/sys/acpi/lenovo")) throw new SecurityException();
		}
	}

	public static void main(String[] args) throws UnsupportedEncodingException {
		if (args.length <= 1) {
			direct = (args.length > 0);
	
			SecurityManager sm = new SodiumSecurity();
			System.setSecurityManager(sm);
	
			System.out.println("*** LIBSODIUM JNI TEST " + (direct ? "(direct)" : "") + " ***");
			
			SodiumUser userAlice = new SodiumUser("Alice");
			SodiumUser userBob = new SodiumUser("Bob");
	
			String msgBob = "Meet me in the pub in 10mins";
	
			SodiumMessage msg = userBob.encrypt(msgBob, userAlice);
			userAlice.decrypt(msg, userBob);
		} else if (args.length <= 2) {
			System.out.println("*** MATRIX SQUARING TEST ***");
			
			for (int i = 1; i <= 100; i++) {
				System.err.println("size = " + i);
				float[] matrix1 = generateMatrix(i);
				float[] matrix2 = generateMatrix(i);
				
				for (int j = 0; j < 100; j++) {
					long start = System.nanoTime();
					squareMatrix(i, matrix1, matrix2);
					long afterNative = System.nanoTime();
					squareMatrix_pure(i, matrix2, matrix1);
					long afterPure = System.nanoTime();
					System.out.println(i + ", " + (afterNative - start) + ", " + (afterPure - afterNative));
				}
			}
		} else {
			String path = "/tmp/matrix.log";
			
			for (int i = 0; i < 500; i++) {

				// System.out.println("Security NULL");
				System.setSecurityManager(null);
				long time0_1 = System.nanoTime();
				readAccess(path);
				long time0_2 = System.nanoTime();
				
				// System.out.println("Security 0");
				System.setSecurityManager(new AccessSecurity0());
				long time1_1 = System.nanoTime();
				readAccess(path);
				long time1_2 = System.nanoTime();
	
				// System.out.println("Security 1");
				System.setSecurityManager(new AccessSecurity1());
				long time2_1 = System.nanoTime();
				readAccess(path);
				long time2_2 = System.nanoTime();
	
				// System.out.println("Security 2");
				System.setSecurityManager(new AccessSecurity2());
				long time3_1 = System.nanoTime();
				readAccess(path);
				long time3_2 = System.nanoTime();
	
				// System.out.println("Security 3");
				System.setSecurityManager(new AccessSecurity3());
				long time4_1 = System.nanoTime();
				readAccess(path);
				long time4_2 = System.nanoTime();
	
				// System.out.println("Security 4");
				System.setSecurityManager(new AccessSecurity4());
				long time5_1 = System.nanoTime();
				readAccess(path);
				long time5_2 = System.nanoTime();
				
				System.out.println((time0_2 - time0_1) + ", " + (time1_2 - time1_1) + ", " + (time2_2 - time2_1) + ", " + (time3_2 - time3_1) + ", " + (time4_2 - time4_1) + ", " + (time5_2 - time5_1));
			}
		}
	}
}
