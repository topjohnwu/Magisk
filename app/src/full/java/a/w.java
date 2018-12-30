package a;

import android.content.Context;

import com.topjohnwu.magisk.services.DelegateWorker;

import java.lang.reflect.ParameterizedType;

import androidx.annotation.NonNull;
import androidx.work.Worker;
import androidx.work.WorkerParameters;

public class w<T extends DelegateWorker> extends Worker {

    /* Wrapper class to workaround Proguard -keep class * extends Worker */

    private T base;

    @SuppressWarnings("unchecked")
    w(@NonNull Context context, @NonNull WorkerParameters workerParams) {
        super(context, workerParams);
        try {
            base = ((Class<T>) ((ParameterizedType) getClass().getGenericSuperclass())
                    .getActualTypeArguments()[0]).newInstance();
        } catch (Exception ignored) {}
    }

    @NonNull
    @Override
    public Result doWork() {
        if (base == null)
            return Result.failure();
        return base.doWork();
    }
}
